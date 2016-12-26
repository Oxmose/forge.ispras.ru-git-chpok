#******************************************************************
#
# Institute for System Programming of the Russian Academy of Sciences
# Copyright (C) 2016 ISPRAS
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, Version 3.
#
# This program is distributed in the hope # that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU General Public License version 3 for more details.
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

import os.path
import json
from elftools.elf.elffile import ELFFile

FLAG_ALLOC = 1<<1
FLAG_WRITE = 1
FLAG_EXECUTE = 1<<2
ALL_FLAG = [FLAG_EXECUTE|FLAG_ALLOC, FLAG_WRITE|FLAG_ALLOC, FLAG_ALLOC]

flag = {}
flag[6] = 'RX'
flag[3] = 'RW'
flag[2] = 'RO'

def name_f(addr_part):
    return os.path.join(addr_part,"part.elf")

def full_sizes(flag,addr_part):
    sizes = 0
    with open(name_f(addr_part), 'rb') as f:
        elffile = ELFFile(f)
        for section in elffile.iter_sections():
            if section['sh_flags'] == flag:
                sizes = sizes + section['sh_size']
    return sizes

def divide_by_TLB_entry(pid,size_pid_for_flag,addr_part):
    for flag in ALL_FLAG:
        size_area = full_sizes(flag,addr_part)
        size_pid_for_flag.append([
                size_area,
                pid,
                flag])
    return size_pid_for_flag

def find_va(flag,addr_part):
    with open(name_f(addr_part), 'rb') as f:
        elffile = ELFFile(f)
        for section in elffile.iter_sections():
            if section['sh_flags'] == flag:
                return section['sh_addr']


def divided_by_flag_and_pid(addr_parts):
    size_pid_for_flag=[]
    for i in range(len(addr_parts)):
        size_pid_for_flag = divide_by_TLB_entry(i+1, 
                                                size_pid_for_flag,addr_parts[i])
    return size_pid_for_flag

def create_json_entry_section(size_pid_for_flag,addr_parts):
    
    entries={}
    for entry in size_pid_for_flag:
        entries[entry[1]] = []
    
    for entry in size_pid_for_flag:
        d = {"virtual_addres" : hex(find_va(entry[2],addr_parts[entry[1]-1])), 
             "flag" : flag[entry[2]], 
             "size" : entry[0],
             "flag_continuous" : 'No'
             }
        entries[entry[1]].append(d)
    with open('entry1.json','w') as f:
        f.write(json.dumps(entries , indent = 4))

def main(addr_parts):
    size_pid_for_flag = divided_by_flag_and_pid(addr_parts)
    create_json_entry_section(size_pid_for_flag,addr_parts)

        
#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()