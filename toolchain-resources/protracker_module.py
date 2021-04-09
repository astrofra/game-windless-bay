#!/usr/bin/python

"""
For license, see gpl-3.0.txt
"""
from PIL import Image
import os
import argparse
import math
import struct
import shutil
import time
from hostplatform import host_platform

folder_tmp = "_temp_/"

def write_module_pak(srcfile, destfile, packer_name="nrv2x"):
	# destfile = destfile + ".pak"
	tmpfile = os.path.join(folder_tmp, os.path.basename(srcfile))
	if os.path.exists(tmpfile):
		os.remove(tmpfile)
	print("write_module_pak(" + srcfile + ", " + destfile + ")")
	print("tmpfile = " + tmpfile)

	original_filesize = os.path.getsize(srcfile)
	print(original_filesize)

	if packer_name == "shrinkler":
		if host_platform() == 'win':
			result = os.popen('packer_shrinkler45\\win\\Shrinkler.exe -d -i 9' + ' "' + srcfile + '"' + ' ' + ' "' + tmpfile + '"').read()
		if host_platform() == 'osx':
			result = os.popen('packer_shrinkler45/osx/Shrinkler -d -i 9' + ' "' + srcfile + '"' + ' ' + ' "' + tmpfile + '"').read()
	elif packer_name == "miniz":
		if host_platform() == 'win':
			result = os.popen('packer_miniz\\win\\miniz.exe -l10 c' + ' "' + srcfile + '"' + ' ' + ' "' + tmpfile + '"').read()
		if host_platform() == 'osx':
			result = os.popen('packer_miniz/osx/miniz -l10 c' + ' "' + srcfile + '"' + ' ' + ' "' + tmpfile + '"').read()
	elif packer_name == 'nrv2x':
		if host_platform() == 'win':
			result = os.popen('packer_nrv2x\\win\\nrv2x.exe -es -k' + ' -o "' + tmpfile + '"' + ' ' + '"' + srcfile + '"').read()
		if host_platform() == 'osx':
			raise NameError('nrv2x not found on OSX!')

	pack_data_list = []
	with open(tmpfile, "rb") as infile:
		while True:
			b = infile.read(1)
			if b:
				b = struct.unpack('b', b)[0]
				pack_data_list.append(b)
			else:
				break

		data_list = pack_data_list

	with open(os.path.join(folder_tmp, os.path.basename(destfile) + '.pak'), 'wb') as c_outfile:
		#	File Header
		c_outfile.write('PTPK'.encode())
		c_outfile.write(original_filesize.to_bytes(4, byteorder='big', signed=False))

		if packer_name == "shrinkler":
			c_outfile.write('SHRK'.encode())
		elif packer_name == "miniz":
			c_outfile.write('MINZ'.encode())
		elif packer_name == 'nrv2x':
			c_outfile.write('NRV2'.encode())

		c_outfile.write('SIZE'.encode())

		filesize = len(pack_data_list)
		print(str(filesize))
		c_outfile.write(filesize.to_bytes(4, byteorder='big', signed=False))

		for _byte in pack_data_list:
			c_outfile.write(_byte.to_bytes(1, byteorder='big', signed=True))

	shutil.copy(os.path.join(folder_tmp, os.path.basename(destfile) + '.pak'), destfile + '.pak')

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Protracker Module converter')
	parser.add_argument('--modfile', required=True)
	parser.add_argument('--destfile', required=True)
	parser.add_argument('--packer', required=False)

	args = parser.parse_args()
	# mod = Image.open(args.modfile)
	if args.packer is not None:
		write_module_pak(args.modfile.replace('\\', '/'), args.destfile.replace('\\', '/'), args.packer.lower())
	else:
		write_module_pak(args.modfile.replace('\\', '/'), args.destfile.replace('\\', '/'))

# write_module_pak("resources/mus_rapanui.mod", "../game/assets\mus_rapanui")