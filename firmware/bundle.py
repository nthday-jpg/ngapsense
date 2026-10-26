import os
import re

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

ENTRY_FILE = os.path.join(SCRIPT_DIR, "main.ino")
OUTPUT_FILE = os.path.join(SCRIPT_DIR, "dist", "wokwi_sketch.ino")

INCLUDE_REGEX = re.compile(r'^\s*#include\s*["\']([^"\']+)["\']')
SYSTEM_INCLUDE_REGEX = re.compile(r'^\s*#include\s*<([^>]+)>')

# System includes (angle brackets) are collected here and written before any
# code. The Arduino build inserts generated function prototypes right after the
# top-level includes, so a type like PubSubClient must be known before the first
# function definition or the prototypes fail to compile.
system_includes = []

def process_file(file_path, processed_files):
    if file_path in processed_files:
        return ""
    
    processed_files.add(file_path)
    output_lines = []
    
    if not os.path.exists(file_path):
        print(f"File not found: {file_path}")
        return ""

    with open(file_path, "r", encoding="utf-8") as f:
        for line in f:
            system_match = SYSTEM_INCLUDE_REGEX.match(line)
            if system_match:
                include_line = line.strip()
                if include_line not in system_includes:
                    system_includes.append(include_line)
                continue

            match = INCLUDE_REGEX.match(line)
            if match:
                included_filename = match.group(1)
                base_dir = os.path.dirname(file_path)
                included_path = os.path.normpath(os.path.join(base_dir, included_filename))
                
                included_code = process_file(included_path, processed_files)
                output_lines.append(f"\n// --- BEGIN INCLUDED FILE: {included_filename} ---\n")
                output_lines.append(included_code)
                if included_code and not included_code.endswith("\n"):
                    output_lines.append("\n")
                output_lines.append(f"// --- END INCLUDED FILE: {included_filename} ---\n")
            else:
                output_lines.append(line)
                
    return "".join(output_lines)

def bundle():
    os.makedirs(os.path.dirname(OUTPUT_FILE), exist_ok=True)
    system_includes.clear()
    processed = set()
    bundled_code = process_file(ENTRY_FILE, processed)

    include_block = ""
    if system_includes:
        include_block = "\n".join(system_includes) + "\n\n"

    bundled_code = include_block + bundled_code
    if bundled_code and not bundled_code.endswith("\n"):
        bundled_code += "\n"
    
    with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
        f.write(bundled_code)
        
    print(f"Successfully bundled code into {OUTPUT_FILE}")

if __name__ == "__main__":
    bundle()