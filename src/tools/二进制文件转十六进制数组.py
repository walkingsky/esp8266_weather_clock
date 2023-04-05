#!/usr/bin python3
# -*- coding: utf-8 -*-

import os
import sys
import struct

def read_data_from_binary_file(filename, list_data):
    f = open(filename, 'rb')
    f.seek(0, 0)
    while True:
        t_byte = f.read(1)
        if len(t_byte) == 0:
            break
        else:
            list_data.append("0x%.2x" % ord(t_byte))
    f.close()

def write_data_to_text_file(filename, list_data, data_num_per_line):
    f_output = open(filename, 'w+')
    if ((data_num_per_line <= 0) or data_num_per_line > len(list_data)):
        data_num_per_line = 16
        print('data_num_per_line out of range,use default value\n')
    f_output.write('    ')
    for i in range(0,len(list_data)):
        if ( (i != 0) and (i % data_num_per_line == 0)):
            f_output.write('\n    ')
            f_output.write(list_data[i]+', ')
        elif (i + 1)== len(list_data):
            f_output.write(list_data[i])
        else:
            f_output.write(list_data[i]+', ')
    f_output.write('\n')
    f_output.close()

def bin_2_c_array(src,dest):
    input_f = src
    output_f = dest
    data_num_per_line = 16
    list_data = []
    read_data_from_binary_file(input_f, list_data)
    write_data_to_text_file(output_f, list_data, data_num_per_line)

if __name__ == '__main__':
        bin_2_c_array(sys.argv[1], sys.argv[2])