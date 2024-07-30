#!/bin/python
import re
if __name__ == "__main__":
    out = ""
    with open("syscalls.txt", "r") as f:
        for line in f.readlines():
            m = re.fullmatch(r"(\d+)(\s+\S+)*\s+(\S+)", line.strip())
            syscall_nr = m.groups()[0]
            syscall_name = m.groups()[-1].removeprefix("sys_")
            out += f"case {syscall_nr}: return \"{syscall_name}\";\n"

    print(out)
    with open("syscalls_cpp.txt", "w") as f:
        f.write(out)
