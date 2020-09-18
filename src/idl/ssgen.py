#!/usr/bin/python3

import sys
import getopt
from pathlib import Path
import re

type_map = {
    "bool": ["bool", 1],
    "char": ["char", 1],
    "f32": ["float", 4],
    "f64": ["double", 8],
    "i8": ["int8_t", 1],
    "i16": ["int16_t", 2],
    "i32": ["int32_t", 4],
    "i64": ["int64_t", 8],
    "u8": ["uint8_t", 1],
    "u16": ["uint16_t", 2],
    "u32": ["uint32_t", 4],
    "u64": ["uint64_t", 8],
    "uchar": ["unsigned char", 1],
}

# reference: https://en.cppreference.com/w/cpp/language/identifiers
IDENT_REGEX = re.compile("^[A-Za-z_\u00a8\u00aa\u00ad\u00af\u00b2-\u00b5\u00b7-\u00ba"
    "\u00bc-\u00be\u00c0-\u00d6\u00d8-\u00f6\u00f8-\u02ff\u0370-\u167f\u1681-\u180d\u180f-\u1dbf\u1e00-\u1fff\u200b-\u200d"
    "\u202a-\u202e\u203f-\u2040\u2054\u2060-\u20cf\u2100-\u218f\u2460-\u24ff\u2776-\u2793\u2c00-\u2dff\u2e80-\u2fff"
    "\u3004-\u3007\u3021-\u302f\u3031-\ud7ff\uf900-\ufd3d\ufd40-\ufdcf\ufdf0-\ufe1f\ufe30-\ufe44\ufe47-\ufffd]"
    "[A-Za-z0-9_\u00a8\u00aa\u00ad\u00af\u00b2-\u00b5\u00b7-\u00ba"
    "\u00bc-\u00be\u00c0-\u00d6\u00d8-\u00f6\u00f8-\u167f\u1681-\u180d\u180f-\u1fff\u200b-\u200d"
    "\u202a-\u202e\u203f-\u2040\u2054\u2060-\u218f\u2460-\u24ff\u2776-\u2793\u2c00-\u2dff\u2e80-\u2fff"
    "\u3004-\u3007\u3021-\u302f\u3031-\ud7ff\uf900-\ufd3d\ufd40-\ufdcf\ufdf0-\ufe44\ufe47-\ufffd]*$")

class Record:
    def __init__(self):
        self.struct_name = ""
        self.comments = []
        self.fields = []

class RecordField:
    def __init__(self):
        self.field_name = ""
        self.comments = []
        self.field_type = ""
        self.field_width = 0
        self.array_size = 0

    def cpp_type(self, ctor=False):
        if self.array_size:
            return f"{'const ' if ctor else ''}std::array<{self.field_type}, {self.array_size}>{' &' if ctor else ''}"
        return self.field_type
    
    def total_width(self):
        if self.array_size:
            return self.field_width * self.array_size
        return self.field_width

def help():
    print("Generates SeriStruct records from IDL\n")
    print("ssgen.py -i <inputfile> -o <outputdir> [--guard] [-n|--namespace <namespace>] [--ext <extension>]\n")
    print("    inputfile    Input IDL file")
    print("    ouputdir     Path to put generated .hpp files")
    print("    --guard      Use DEFINE guard rather than pragma once")
    print("    namespace    A namespace for qualifying the generated records")
    print("    extension    The extension for generated header files (defaults to .gen.hpp)")


def error(msg):
    print(msg, file=sys.stderr)
    sys.exit(2)

def is_valid_cpp_identifier(ident):
    if ident:
        return IDENT_REGEX.match(ident) is not None
    return False

def parse_field(field):
    fields = field.split()
    if len(fields) != 2:
        return None
    array_matches = re.match(r"^([^\[]+)\[(\d+)\]$", fields[1])
    if array_matches:
        groups = array_matches.groups()
        if groups[0] in type_map:
            record_field = RecordField()
            record_field.field_name = fields[0]
            record_field.field_type = type_map[groups[0]][0]
            record_field.field_width = type_map[groups[0]][1]
            record_field.array_size = int(groups[1])
            return record_field
    elif fields[1] in type_map:
        record_field = RecordField()
        record_field.field_name = fields[0]
        record_field.field_type = type_map[fields[1]][0]
        record_field.field_width = type_map[fields[1]][1]
        return record_field
    return None


# Parse arguments
inputfile = ""
outputdir = ""
guard = False
namespace = ""
hpp_ext = ".gen.hpp"

try:
    opts, args = getopt.getopt(sys.argv[1:], "h?i:o:n:", ["help", "guard", "namespace=", "ext="])
except getopt.GetoptError:
    help()
    sys.exit(2)
for opt, arg in opts:
    if opt in ("-h", "-?", "--help"):
        help()
        sys.exit(0)
    elif opt == "-i":
        inputfile = arg
    elif opt == "-o":
        outputdir = arg
    elif opt == "--guard":
        guard = True
    elif opt in ("-n", "--namespace"):
        namespace = arg
    elif opt == "--ext":
        hpp_ext = arg
        if hpp_ext[0] != ".":
            print("Warning: extension supplied does not start with period (.)", file=sys.stderr)

if inputfile == "" or outputdir == "":
    error("Both inputfile and outputdir are required")

print(f"Reading IDL from: {inputfile}")
print(f"Writing generated files to: {outputdir}")

# Confirm input and output exist
inputpath = Path(inputfile)
if not (inputpath.is_file() and inputpath.exists()):
    error(f"Input file {inputfile} does not exist")

outputpath = Path(outputdir)
if outputpath.exists() and outputpath.is_file():
    error(f"Output dir {outputdir} specifies a file")
outputpath.mkdir(parents=True, exist_ok=True)

# Open input IDL file for reading and parse
parsed_idl = []
try:
    with open(inputpath) as fd:
        comments = []
        line_no = 0

        for line in fd:
            line_no += 1
            if not line.isspace():
                if line[0].isspace():
                    error(
                        f"Unexpected whitespace in {inputfile} at line {line_no}")
                line = line.rstrip()
                if line[0] == '"' and line[-1] == '"':
                    comments.append(line[1:-1])
                elif line[-1] == ':':
                    record = Record()
                    record.struct_name = line[:-1]
                    if not is_valid_cpp_identifier(record.struct_name):
                        error(f"Invalid identifier {record.struct_name} in {inputfile} at {line_no}")
                    record.comments = comments.copy()
                    comments.clear()
                    for line in fd:
                        line_no += 1
                        if line.isspace():
                            break
                        if not line[0].isspace():
                            error(f"Expected whitespace in {inputfile} at line {line_no}")
                        line = line.strip()
                        if line[0] == '"' and line[-1] == '"':
                            comments.append(line[1:-1])
                        else:
                            field = parse_field(line.strip())
                            if field is None:
                                error(
                                    f"Syntax error in {inputfile} at line {line_no}")
                            if not is_valid_cpp_identifier(field.field_name):
                                error(f"Invalid identifier {record.struct_name}.{field.field_name} in {inputfile} at {line_no}")
                            field.comments = comments.copy()
                            comments.clear()
                            record.fields.append(field)
                    if len(comments):
                        error(f"Orphaned comments in {inputfile} at line {line_no}")
                    if len(record.fields) == 0:
                        error(f"Record {record.struct_name} in {inputfile} at line {line_no} has no fields")
                    parsed_idl.append(record)
                else:
                    error(f"Syntax error in {inputfile} at line {line_no}")
        if len(comments):
            error(f"Orphaned comments in {inputfile} at line {line_no}")
except IOError as e:
    error(f"File error: {e}")

for idl in parsed_idl:
    hpp = outputpath.joinpath(f"{idl.struct_name}{hpp_ext}")
    print(f"Writing {idl.struct_name} to {hpp}...")
    try:
        with open(hpp, mode="w") as fd:
            # Write opener
            if guard:
                fd.write(f"#ifndef SERISTRUCT_RECORD_{idl.struct_name.upper()}_HPP\n")
                fd.write(f"#define SERISTRUCT_RECORD_{idl.struct_name.upper()}_HPP\n\n")
            else:
                fd.write("#pragma once\n")
            fd.write("#include <SeriStruct.hpp>\n\n")

            if namespace:
                fd.write(f"namespace {namespace}\n{{\n")
            fd.write("using SeriStruct::Record;\n\n")
            # Write comments and class open
            if len(idl.comments):
                fd.write("/**\n")
                for comment in idl.comments:
                    fd.write(f" * {comment}\n")
                fd.write(" */\n")
            fd.write(f"""class {idl.struct_name} : public Record
{{
public:
    {idl.struct_name}(""")
            # Write constructor arguments and body
            for (idx, field) in enumerate(idl.fields):
                if idx > 0:
                    fd.write(", ")
                fd.write(f"{field.cpp_type(ctor=True)} {field.field_name}")
            fd.write(")\n        : Record{}\n    {\n")
            fd.write("        alloc(buffer_size);\n")
            for field in idl.fields:
                fd.write(f"        assign_buffer(offset_{field.field_name}, {field.field_name});\n")
            fd.write("    }\n")

            # Write remaining boilerplate constructors
            fd.write(f"""    {idl.struct_name}(std::istream &istr, const size_t read_size) : Record{{istr, read_size, buffer_size}} {{}}
    {idl.struct_name}(const unsigned char *buffer, const size_t buffer_size) : Record{{buffer, buffer_size, {idl.struct_name}::buffer_size}} {{}}
    {idl.struct_name}(const {idl.struct_name} &other) : Record{{other}} {{}}
    {idl.struct_name}({idl.struct_name} &&other) : Record{{std::move(other)}} {{}}
    ~{idl.struct_name}() {{}}
    {idl.struct_name} &operator=(const {idl.struct_name} &other)
    {{
        Record::operator=(other);
        return *this;
    }}\n\n""")
            
            # Write field getters
            for field in idl.fields:
                if len(field.comments):
                    fd.write("    /**\n")
                    for comment in field.comments:
                        fd.write(f"     * {comment}\n")
                    fd.write("     */\n")
                fd.write(f"    inline {field.cpp_type()} &{field.field_name}() const {{ return buffer_at<{field.field_type}")
                if field.array_size:
                    fd.write(f", {field.array_size}")
                fd.write(f">(offset_{field.field_name}); }}\n")
            
            fd.write("\nprivate:\n")
            # Calculate offsets and write private fields
            current_offset = 0
            previous_field = None
            for field in idl.fields:
                fd.write(f"    static constexpr size_t offset_{field.field_name} = ")
                if previous_field is None:
                    fd.write("0")
                else:
                    padding = (field.field_width - (current_offset % field.field_width)) % field.field_width
                    if padding:
                        current_offset += padding
                        fd.write(f"{padding} /* padding */ + ")
                    fd.write(f"offset_{previous_field.field_name} + sizeof({previous_field.cpp_type()})")
                fd.write(";\n")
                current_offset += field.total_width()
                previous_field = field
            fd.write(f"    static constexpr size_t buffer_size = offset_{previous_field.field_name} + sizeof({previous_field.cpp_type()});\n")
            
            # Write close of class
            fd.write("};\n")
            if namespace:
                fd.write(f"}} /* {namespace} */\n")
            if guard:
                fd.write("\n#endif\n")
    except IOError as e:
        error(f"File error: {e}")