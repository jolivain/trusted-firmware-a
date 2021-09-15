#!/usr/bin/env python3
import cmakeast
import glob
import os

from pprint import pprint

TFA_ROOT = "./" #"~/tfa/trusted-firmware-a/"
TFA_CMAKE_DOCS = TFA_ROOT + "docs/cmake/"
TFA_CMAKE_VARS = TFA_CMAKE_DOCS + "manual/variable/"

CMAKE_FILENAME = "CMakeLists.txt"
EXCLUSIONS_FILENAME = os.path.join(
                        os.path.dirname(
                          os.path.realpath(__file__)),
                          "cmake_configvars_exclusions.txt")

def contents_from_func_call(func_call, vprint):
    varname = func_call.arguments[1].contents
    vprint(" - " +  varname)
    return varname

def lineno_from_func_call(func_call):
    return func_call.arguments[1].line

def filter_func_calls(statement, func):
    if isinstance(statement, cmakeast.ast.FunctionCall):
        return (statement.name == func)
    return False

# Use cmakeast and dict/list comprehension to get all variables in a dict
def parse_cmake_file(path, vprint):
    result = {}
    vprint("Parsing " + path)
    with open(path) as f:
        contents = f.read()
        tree = cmakeast.ast.parse(contents).statements
        result = { contents_from_func_call(fc, vprint) : path +
                                       ":" + str(lineno_from_func_call(fc))
                      for fc in list(
                                 filter(
                                        lambda x: filter_func_calls(x, "arm_config_option"),
                                        tree
                                       ))
                 }
    return result

def os_strip(path):
    return os.path.splitext(os.path.basename(path))[0]

def read_exclusions(vprint):
    vprint("---------------------------")
    vprint("Getting variable exclusions")
    vprint("---------------------------")
    vprint("Reading exclusions file:\n " + os.path.relpath(EXCLUSIONS_FILENAME))
    vprint("Found exclusions:")
    ret = []
    with open(EXCLUSIONS_FILENAME) as f:
        ret = f.read().splitlines()
    vprint(" - " + "\n - ".join(ret))
    return ret

## Set Operations ##

def not_documented(defined, documented, exclusions):

    not_doc = list(set(defined.keys()) - set(documented.keys()) - set(exclusions))
    not_doc.sort()

    ret = { k : defined[k] for k in not_doc }
    return ret

def not_defined(defined, documented, exclusions):
    not_def = list(set(documented.keys()) - set(defined.keys()) - set(exclusions))
    not_def.sort()

    ret = { k : documented[k] for k in not_def }
    return ret

# Entry Point for include
def check_configvars(verbose=False):

    def vprint(msg):
        if verbose:
            print("(v) " + msg.replace("\n","\n(v) "))

    cmake_vars = {}

    vprint("----------------------------------------")
    vprint("Getting Variables Defined in Source Tree")
    vprint("----------------------------------------")

    # Glob pattern for all CMake files in tree from root of TF-A source
    cmake_search_path = TFA_ROOT + "**/" + CMAKE_FILENAME
    vprint(CMAKE_FILENAME + " search path:\n " + cmake_search_path)

    cmake_files = glob.glob(cmake_search_path, recursive=True)

    # Parse each CMake file using cmakeast
    for path in cmake_files:
        cmake_vars.update(parse_cmake_file(path, vprint))

    vprint("----------------------------")
    vprint("Getting Documented Variables")
    vprint("----------------------------")

    docfile_search_path = TFA_CMAKE_VARS + "**/*.rst"
    vprint("Documentation search path:\n " + docfile_search_path)
    vprint("Found variable documentation files:")

    docfiles = glob.glob(docfile_search_path, recursive=True)
    doc_vars = { os_strip(rst) : rst  for rst in docfiles}

    sorted_paths = [ x.replace(TFA_CMAKE_VARS,"") for x in list(doc_vars.values()) ]
    sorted_paths.sort()

    vprint(" - " + "\n - ".join(sorted_paths))

    excl = read_exclusions(vprint)

    not_doc = not_documented(cmake_vars, doc_vars, excl)
    not_def = not_defined(cmake_vars, doc_vars, excl)
    return { "Not Documented" : not_doc, "Not Defined" : not_def }

def main():
    error_vars = check_configvars()
    pprint(error_vars)

if __name__ == "__main__":
    main()

