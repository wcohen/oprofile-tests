#!/usr/bin/env python

"""
 Copyright 2005 Philippe Elie <phil.el@wanadoo.fr>

 Utility to roughly check than opimport is working, it convert sample files
 to a temporary directory w/o changing the abi and compare the output of
 opreport -l --details and opreport -c for the two database. Using the same abi
 for both samples files means the conversion code itself is not very well
 tested.


 This file can be used also to import a session to the current abi with a two
 lines python script:
 
   import check_opimport
   check_opimport.import_files_session('/var/lib/oprofile/', '/var/lib/oprofile/samples/current/', 'output_dir')

 The first parameter gives the directory where live the abi file.
 

 TODO:

  Use distinct abi and convert the files twice, abi A --> abi B --> abi A
  then compare the output, it needs changes in opimport to allow to specify
  the native abi format.

"""

version = '0.1'
oprofile_dir = '/var/lib/oprofile/'
oprofile_samples_dir = oprofile_dir + 'samples/current'
# FIXME
output_dir = './phe-temp'

import sys
import os
import popen2

# in case psyco jit is available, if it's not install it or take a cup of tea
# whilst the script run.
try:
    import psyco
    psyco.full()
except ImportError:
    pass

def generate_filename(top_dir):
    for root, dirs, files in os.walk(top_dir):
        for f in files:
            yield (root, f)


def import_files_session(oprofile_dir, oprofile_samples_dir, output_dir):
    for (directory, filename) in generate_filename(oprofile_samples_dir):
        if not os.access(output_dir + directory, os.F_OK):
            os.makedirs(output_dir + directory)

        input_file = directory + '/' + filename
        output_file = output_dir + input_file
        cmd_line = 'opimport -a ' + oprofile_dir + 'abi -f'
        cmd_line += ' -o ' + output_file + ' ' + input_file
        if os.access(output_file, os.F_OK):
            print 'output file already exist:', output_file
            print 'since opimport accumulate result into output file you want to cleanup the output dir:', output_dir, 'first'
            sys.exit(1)
        child = popen2.Popen4(cmd_line)
        fd = child.fromchild
        for t in fd.readlines():
            print t.strip('\n')
        if child.wait() != 0:
            print cmd_line + " return: " + str(child.wait())
            sys.exit(1)


def run_opreport(options, session):
    data = ''
    cmd_line = 'opreport ' + options + ' session:' + session
    child = popen2.Popen4(cmd_line)
    fd = child.fromchild
    for t in fd.readlines():
        # CPU Mhz can be set to zero after an import
        if not t.startswith('CPU: '):
            data += t
    if child.wait() != 0:
        print cmd_line + " return: " + str(child.wait())
        sys.exit(1)
    return data


def opreport_diff(options, oprofile_samples_dir, output_dir):
    data1 = run_opreport(options, oprofile_samples_dir)
    data2 = run_opreport(options, output_dir + oprofile_samples_dir)
    if data1 != data2:
        print 'something feel bad with options: ' + options
        print 'try\n' + 'opreport ' + options + ' session:' + oprofile_samples_dir + ' > temp1'
        print 'opreport ' + options + ' session:' + output_dir + oprofile_samples_dir + ' > temp2'
        print 'diff -u temp1 temp2'
    else:
        print 'opreport ' + options + ' output are identical'


if __name__ == '__main__':
    import_files_session(oprofile_dir, oprofile_samples_dir, output_dir)
    opreport_diff('-l --details', oprofile_samples_dir, output_dir)
    opreport_diff('-c', oprofile_samples_dir, output_dir)
