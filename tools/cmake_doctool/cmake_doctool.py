#!/usr/bin/env python3

import argparse
import sys

from cmake_configvars import check_configvars
from tabulate import tabulate

# Handle command line args and return args Namespace
def parse_args():
    parser = argparse.ArgumentParser(
        description="Check the Documentation for the Trusted Firmware A CMake Build System"
    )

    parser.add_argument(
        "-v", "--verbose", help="increase output verbosity", action="store_true"
    )
    parser.add_argument(
        "--check-vars",
        default=True,
        help="validate that all variables are documented, and all documented variables are still in use. (default)",
        action="store_true",
    )
    parser.add_argument(
        "-o",
        help="output to a text file",
        metavar="OUTFILE",
        dest="outfile",
        default="",
    )
    return parser.parse_args()


# Print the output formatted as a table. Output stream may be a file
def print_configvars(d):
    for k in d:
        if not d[k].items():
            # Nice output for empty list
            d[k].update({"None": ""})
        headers = ["Variable " + k, "Origin"]
        print(tabulate(d[k].items(), headers=headers))
        print("")


def main():
    args = parse_args()

    if args.check_vars:

        # Set target of print()
        if args.outfile:
            original_stdout = sys.stdout
            with open(args.outfile, "w") as f:
                sys.stdout = f
                d = check_configvars(args.verbose)
                print_configvars(d)
                sys.stdout = original_stdout
        else:
            print("")
            d = check_configvars(args.verbose)
            print_configvars(d)


if __name__ == "__main__":
    main()
