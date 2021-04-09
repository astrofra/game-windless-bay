#!/usr/bin/python

"""
For license, see gpl-3.0.txt
"""
import os
import argparse
import math
import struct
import shutil
import subprocess
import wave
from hostplatform import host_platform

folder_tmp = "_temp_/"

# File structure
# SMPK (4 bytes) 'Sample Packer' Header
# SIZE (4 bytes) 'Size information block' header
# ____ (4 bytes) Size of the original uncompressed & decoded sample
# ADPC|8SVX (4 bytes) Encoder type. ADPC = ADPCM, 8SVX = Amiga 8bits IFF sample.
# SIZE (4 bytes) 'Size information block' header
# ____ (4 bytes) Size of the encoded sample
# MINZ|SHRK|D68K (4 bytes) Compressor type. MINZ = Tinfl Miniz, SHRK = Shrinkler, D68K = Doynamite68k
# SIZE (4 bytes) 'Size information block' header
# ____ (4 bytes) Size of the compressed block

def write_sample_pak(srcfile, destfile, frequency, packer_name, encoder_name):
	# destfile = destfile + ".pak"
	if packer_name is None:
		packer_name = "miniz"
	if encoder_name is None:
		encoder_name = '8svx'
	tmpfile = os.path.join(folder_tmp, os.path.basename(srcfile))
	print("write_sample_pak(" + srcfile + ", " + destfile + ")")

	if not os.path.exists(folder_tmp):
		os.mkdir(folder_tmp)

	original_filesize = -1
	with wave.open(srcfile, mode='rb') as _in_wave:
		# original_filesize = _in_wave.getnframes() // _in_wave.getsampwidth()

		if frequency is None or str(frequency) == "-1" or (str(frequency).isnumeric() and int(frequency) < 0):
			frequency = _in_wave.getframerate()
		else:
			frequency = int(frequency)

	# print("[original sample size]:" + str(original_filesize))

	if encoder_name == '8svx':
		# generate a temporary WAV in order to get the final number of sample
		if host_platform() == 'win':
			cmd_line = 'sox\\sox.exe'
		if host_platform() == 'osx':
			cmd_line = 'sox'
		cmd_line += ' "' + srcfile + '"'
		cmd_line += ' -D -r' + str(frequency) + ' -b 8 '
		cmd_line += '"' + tmpfile.replace('.wav', '_8Khz.wav') + '"'
		process = subprocess.Popen(cmd_line, stdout=subprocess.PIPE, shell=True)
		print(cmd_line)
		for line in iter(process.stdout.readline, b''):  # replace '' with b'' for Python 3
			print(line)

		# read the sample size from the temporary wav file
		with wave.open(tmpfile.replace('.wav', '_8Khz.wav'), mode='rb') as _in_wave:
			original_filesize = _in_wave.getnframes() * _in_wave.getsampwidth()

		print("[original sample size]:" + str(original_filesize))

		# convert WAV to 8SVX
		if host_platform() == 'win':
			cmd_line = 'sox\\sox.exe'
		if host_platform() == 'osx':
			cmd_line = 'sox'
		cmd_line += ' "' + srcfile + '"'
		cmd_line += ' -D -r' + str(frequency) + ' -e signed -b 8 '
		cmd_line += '"' + tmpfile.replace('.wav', '.raw') + '"'
		process = subprocess.Popen(cmd_line, stdout=subprocess.PIPE, shell=True)
		print(cmd_line)
		for line in iter(process.stdout.readline, b''):  # replace '' with b'' for Python 3
			print(line)

		srcfile = tmpfile.replace('.wav', '.raw')

	if encoder_name == "adpcm":
		if host_platform() == 'win':
			cmd_line = 'sox\\sox.exe'
		if host_platform() == 'osx':
			cmd_line = 'sox'
		cmd_line += ' "' + srcfile + '"'
		cmd_line += ' -D -r' + str(frequency) + ' -b 8 '
		cmd_line += '"' + tmpfile.replace('.wav', '_8Khz.wav') + '"'
		process = subprocess.Popen(cmd_line, stdout=subprocess.PIPE, shell=True)
		print(cmd_line)
		for line in iter(process.stdout.readline, b''):  # replace '' with b'' for Python 3
			print(line)

		# read the sample size from the temporary wav file
		with wave.open(tmpfile.replace('.wav', '_8Khz.wav'), mode='rb') as _in_wave:
			original_filesize = _in_wave.getnframes() * _in_wave.getsampwidth()

		print("[original sample size]:" + str(original_filesize))

		# adpcm encoder
		cmd_line = None
		if host_platform() == 'win':
			cmd_line = 'adpcm\\ADPCM_Crunch.exe'
		if host_platform() == 'osx':
			raise NameError("ADPCM_Crunch not available on OSX!")

		cmd_line += ' -b4'
		cmd_line += ' "' + tmpfile.replace('.wav', '_8Khz.wav') + '"'
		cmd_line += ' "' + tmpfile.replace('.wav', '.adpcm') + '"'

		process = subprocess.Popen(cmd_line, stdout=subprocess.PIPE, shell=True)
		print(cmd_line)
		for line in iter(process.stdout.readline, b''):  # replace '' with b'' for Python 3
			print(line)

		# adpcm QA
		if host_platform() == 'win':
			cmd_line = 'adpcm\\ADPCM_Decrunch.exe'
		if host_platform() == 'osx':
			raise NameError("ADPCM_Decrunch not available on OSX!")

		cmd_line += ' -w'
		cmd_line += ' "' + tmpfile.replace('.wav', '.adpcm') + '"'
		cmd_line += ' "' + tmpfile.replace('.wav', '_adpcm_QA.wav') + '"'

		process = subprocess.Popen(cmd_line, stdout=subprocess.PIPE, shell=True)
		print(cmd_line)
		for line in iter(process.stdout.readline, b''):  # replace '' with b'' for Python 3
			print(line)

		srcfile = tmpfile.replace('.wav', '.adpcm')

	# get the size of the encoded sample
	encoded_filesize = os.path.getsize(srcfile)

	cmd = None
	if packer_name == "shrinkler":
		if host_platform() == 'win':
			cmd = 'packer_shrinkler45\\win\\Shrinkler.exe -d -i 9' + ' "' + srcfile + '"' + ' ' + ' "' + tmpfile + '"'
		if host_platform() == 'osx':
			cmd = 'packer_shrinkler45/osx/Shrinkler -d -i 9' + ' "' + srcfile + '"' + ' ' + ' "' + tmpfile + '"'

	elif packer_name == "miniz":
		if host_platform() == 'win':
			cmd = 'packer_miniz\\win\\miniz.exe -l10 c' + ' "' + srcfile + '"' + ' ' + ' "' + tmpfile + '"'
		if host_platform() == 'osx':
			cmd = 'packer_miniz/osx/miniz -l10 c' + ' "' + srcfile + '"' + ' ' + ' "' + tmpfile + '"'
	# elif packer_name == 'doynamite68k':
	# 	result = os.popen('doynamite68k\\lz.exe -o' + ' "' + tmpfile + '"' + ' ' + '"' + srcfile + '"').read()
	elif packer_name == 'nrv2x':
		if host_platform() == 'win':
			cmd = 'packer_nrv2x\\win\\nrv2x.exe -es' + ' -o "' + tmpfile + '"' + ' ' + '"' + srcfile + '"'
		if host_platform() == 'osx':
			raise NameError('nrv2x not found on OSX!')
	result = os.popen(cmd).read()
	pack_data_list = []
	with open(tmpfile, "rb") as infile:
		while True:
			b = infile.read(1)
			if b:
				b = struct.unpack('b', b)[0]
				pack_data_list.append(b)
			else:
				break

		# data_list = pack_data_list

	with open(os.path.join(folder_tmp, os.path.basename(destfile) + '.pak'), 'wb') as c_outfile:
		#	File Header
		c_outfile.write('SMPK'.encode())
		c_outfile.write('SIZE'.encode())
		c_outfile.write(original_filesize.to_bytes(4, byteorder='big', signed=False))
		c_outfile.write('FREQ'.encode())
		c_outfile.write(frequency.to_bytes(4, byteorder='big', signed=False))

		# write the encoder in the header
		if encoder_name is None or encoder_name == '8svx':
			c_outfile.write('8SVX'.encode())
		elif encoder_name == "adpcm":
			c_outfile.write('ADPC'.encode())
		c_outfile.write('SIZE'.encode())
		c_outfile.write(encoded_filesize.to_bytes(4, byteorder='big', signed=False))

		# write the compressor in the header
		if packer_name == "shrinkler":
			c_outfile.write('SHRK'.encode())
		elif packer_name == "miniz":
			c_outfile.write('MINZ'.encode())
		elif packer_name == "nrv2x":
			c_outfile.write('NRV2'.encode())

		c_outfile.write('SIZE'.encode())

		filesize = len(pack_data_list)
		print("packed size = " + str(filesize))
		c_outfile.write(filesize.to_bytes(4, byteorder='big', signed=False))

		for _byte in pack_data_list:
			c_outfile.write(_byte.to_bytes(1, byteorder='big', signed=True))

	shutil.copy(os.path.join(folder_tmp, os.path.basename(destfile) + '.pak'), destfile + '.pak')

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Audio Sample converter')
	parser.add_argument('--samplefile', required=True)
	parser.add_argument('--destfile', required=True)
	parser.add_argument('--frequency', required=False)
	parser.add_argument('--packer', required=False)
	parser.add_argument('--encoder', required=False)

	args = parser.parse_args()
	write_sample_pak(args.samplefile.replace('\\', '/'), args.destfile.replace('\\', '/'), args.frequency, args.packer, args.encoder)

# python.exe audio_sample.py --samplefile resources/sfx_seagull.wav --destfile ../game/assets\sfx_mouette2 --frequency 6000 --packer miniz --encoder adpcm

# write_sample_pak("resources/sfx_seagull.wav", "../game/assets\sfx_mouette2", 6000, "shrinkler", "adpcm")
# f = 4000
# #
# write_sample_pak("resources/sfx_cat.wav", "../game/assets\sfx_miaou2", 8000, "miniz", "adpcm")
# write_sample_pak("resources/sfx_altos_puzzle_error.wav", "../game/assets\sfx_altos_puzzle_error", f, "shrinkler", "adpcm")
# write_sample_pak("resources/sfx_altos_puzzle_open.wav", "../game/assets\sfx_altos_puzzle_open", f, "shrinkler", "adpcm")
# write_sample_pak("resources/sfx_cat.wav", "../game/assets\sfx_cat", f, "miniz", "adpcm")
# write_sample_pak("resources/sfx_dog.wav", "../game/assets\sfx_dog", f, "shrinkler", "adpcm")
# write_sample_pak("resources/sfx_seagull.wav", "../game/assets\sfx_seagull", f, "shrinkler", "adpcm")
