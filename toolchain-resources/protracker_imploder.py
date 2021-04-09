# Protracker 2.3A Song/Module Format
#
# Offset  Bytes  Description
# ------  -----  -----------
#    0     20    Songname. Remember to put trailing null bytes at the end...
#
# Information for sample 1-31:
#
# Offset  Bytes  Description
# ------  -----  -----------
#   20     22    Samplename for sample 1. Pad with null bytes.
#   42      2    Samplelength for sample 1. Stored as number of words.
#                Multiply by two to get real sample length in bytes.
#   44      1    Lower four bits are the finetune value, stored as a signed
#                four bit number. The upper four bits are not used, and
#                should be set to zero.
#                Value:  Finetune:
#                  0        0
#                  1       +1
#                  2       +2
#                  3       +3
#                  4       +4
#                  5       +5
#                  6       +6
#                  7       +7
#                  8       -8
#                  9       -7
#                  A       -6
#                  B       -5
#                  C       -4
#                  D       -3
#                  E       -2
#                  F       -1
#
#   45      1    Volume for sample 1. Range is $00-$40, or 0-64 decimal.
#   46      2    Repeat point for sample 1. Stored as number of words offset
#                from start of sample. Multiply by two to get offset in bytes.
#   48      2    Repeat Length for sample 1. Stored as number of words in
#                loop. Multiply by two to get replen in bytes.
#
# Information for the next 30 samples starts here. It's just like the info for
# sample 1.
#
# Offset  Bytes  Description
# ------  -----  -----------
#   50     30    Sample 2...
#   80     30    Sample 3...
#    .
#    .
#    .
#  890     30    Sample 30...
#  920     30    Sample 31...
#
# Offset  Bytes  Description
# ------  -----  -----------
#  950      1    Songlength. Range is 1-128.
#  951      1    Well... this little byte here is set to 127, so that old
#                trackers will search through all patterns when loading.
#                Noisetracker uses this byte for restart, but we don't.
#  952    128    Song positions 0-127. Each hold a number from 0-63 that
#                tells the tracker what pattern to play at that position.
# 1080      4    The four letters "M.K." - This is something Mahoney & Kaktus
#                inserted when they increased the number of samples from
#                15 to 31. If it's not there, the module/song uses 15 samples
#                or the text has been removed to make the module harder to
#                rip. Startrekker puts "FLT4" or "FLT8" there instead.
#
# Offset  Bytes  Description
# ------  -----  -----------
# 1084    1024   Data for pattern 00.
#    .
#    .
#    .
# xxxx  Number of patterns stored is equal to the highest patternnumber
#       in the song position table (at offset 952-1079).
#
# Each note is stored as 4 bytes, and all four notes at each position in
# the pattern are stored after each other.
#
# 00 -  chan1  chan2  chan3  chan4
# 01 -  chan1  chan2  chan3  chan4
# 02 -  chan1  chan2  chan3  chan4
# etc.
#
# Info for each note:
#
#  _____byte 1_____   byte2_    _____byte 3_____   byte4_
# /                \ /      \  /                \ /      \
# 0000          0000-00000000  0000          0000-00000000
#
# Upper four    12 bits for    Lower four    Effect command.
# bits of sam-  note period.   bits of sam-
# ple number.                  ple number.
#
# Periodtable for Tuning 0, Normal
#   C-1 to B-1 : 856,808,762,720,678,640,604,570,538,508,480,453
#   C-2 to B-2 : 428,404,381,360,339,320,302,285,269,254,240,226
#   C-3 to B-3 : 214,202,190,180,170,160,151,143,135,127,120,113
#
# To determine what note to show, scan through the table until you find
# the same period as the one stored in byte 1-2. Use the index to look
# up in a notenames table.
#
# This is the data stored in a normal song. A packed song starts with the
# four letters "PACK", and then comes the packed data.
#
# In a module, all the samples are stored right after the patterndata.
# To determine where a sample starts and stops, you use the sampleinfo
# structures in the beginning of the file (from offset 20). Take a look
# at the mt_init routine in the playroutine, and you'll see just how it
# is done.

import os
import shutil
import hashlib
import struct
import wave
import crc16
import subprocess

folder_tmp = "_temp_"
# module_filename = "resources/oxygene2.mod"
chunksize = 4
DEBUG_SAMPLE_COMPRESSION = False


def hex_crc16(str_in):
	return hex(crc16.crc16xmodem(str_in.decode('ansi').encode()))


def write_packed_file(srcfile, destfile, packer_name="shrinkler", header="SMPK", original_filesize = -1):
	# destfile = destfile + ".pak"
	tmpfile = os.path.join(folder_tmp, os.path.basename(srcfile))
	print("write_packed_file(" + srcfile + ", " + destfile + ")")

	if original_filesize == -1:
		original_filesize = os.path.getsize(srcfile)
	print("original_filesize = " + str(original_filesize))

	if packer_name == "shrinkler":
		result = os.popen(
			'packer_shrinkler45\\Windows64\\Shrinkler.exe -d -i 9' + ' "' + srcfile + '"' + ' ' + ' "' + tmpfile + '"').read()
	elif packer_name == "miniz":
		result = os.popen('packer_miniz\\miniz.exe -l10 c' + ' "' + srcfile + '"' + ' ' + ' "' + tmpfile + '"').read()
	elif packer_name == 'nrv2x':
		result = os.popen('packer_nrv2x\\nrv2x.exe -es' + ' -o "' + tmpfile + '"' + ' ' + '"' + srcfile + '"').read()

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

	out_filename = os.path.join(folder_tmp, os.path.basename(destfile) + '.pak')
	file_mode = 'wb'
	if os.path.exists(out_filename):
		file_mode = 'ab'

	with open(out_filename, file_mode) as c_outfile:
		#	File Header
		c_outfile.write(header.encode())
		c_outfile.write('SIZE'.encode())
		c_outfile.write(original_filesize.to_bytes(4, byteorder='big', signed=False))

		if packer_name == "shrinkler":
			c_outfile.write('SHRK'.encode())
		elif packer_name == "miniz":
			c_outfile.write('MINZ'.encode())
		elif packer_name == 'nrv2x':
			c_outfile.write('NRV2'.encode())

		c_outfile.write('SIZE'.encode())

		filesize = len(pack_data_list)
		print("packed_filesize = " + str(filesize))
		c_outfile.write(filesize.to_bytes(4, byteorder='big', signed=False))

		for _byte in pack_data_list:
			c_outfile.write(_byte.to_bytes(1, byteorder='big', signed=True))

	print("shutil.copy(" + os.path.join(folder_tmp, os.path.basename(destfile) + '.pak') + ", " + destfile + '.pak)')
	shutil.copy(os.path.join(folder_tmp, os.path.basename(destfile) + '.pak'), destfile + '.pak')
	print("shutil.copy() : DONE!")


# ------------------------------------------------------------
# READ THEN WRITE ALL SAMPLES --------------------------------
# ------------------------------------------------------------
def write_module_pak(srcfile, destfile, packer_name="shrinkler", adpcm_enable_mask=None):
	print("\n----------------------------")
	module_filename = srcfile
	out_path = os.path.dirname(os.path.abspath(destfile))
	total_module_size = os.path.getsize(module_filename)
	with open(module_filename, "rb") as module_file:
		# ------------------------------------------------------------
		# READ -------------------------------------------------------
		# ------------------------------------------------------------
		print("-- reading protracker module '" + str(srcfile) + "', size = " + str(total_module_size) + " bytes")
		song_name = module_file.read(20).decode('ascii')
		song_name = song_name.rstrip().strip()
		print(song_name)

		song_name = destfile.replace('\\', '/').split('/')[-1]

		# sample definition
		print("------------")
		samples = []
		for sample_idx in range(31):
			sample_name = module_file.read(22).decode('ascii')
			sample_len = int.from_bytes(module_file.read(2), byteorder='big', signed=False) * 2
			finetune = int.from_bytes(module_file.read(1), byteorder='big', signed=False)
			volume = int.from_bytes(module_file.read(1), byteorder='big', signed=False)
			repeat_point = int.from_bytes(module_file.read(2), byteorder='big', signed=False) * 2
			repeat_length = int.from_bytes(module_file.read(2), byteorder='big', signed=False) * 2
			if len(sample_name.replace('\0','').strip()) > 0:
				print("-- reading sample #" + str(sample_idx) + ': ' + sample_name.strip())
				# if sample_len > 0:
				#     # print("\tsample_len = " + str(sample_len) + " byte(s).")
				#     # print("\tfinetune = " + str(finetune))
				#     # print("\tvolume = " + str(volume))
				#     # print("\trepeat_point = " + str(repeat_point))
				#     # print("\trepeat_length = " + str(repeat_length))
				samples.append({"sample_name": sample_name.rstrip().replace('\0', ''), "sample_len": sample_len, "finetune": finetune, "volume": volume, "repeat_point": repeat_point, "repeat_length": repeat_length, "hash": None})
			else:
				samples.append({})

		# song data
		print("------------")
		song_length = int.from_bytes(module_file.read(1), byteorder='big', signed=False)
		old_tracker_len_info = int.from_bytes(module_file.read(1), byteorder='big', signed=False)

		# read song list
		song_pos = []
		max_pattern_idx = -1
		for i in range(128):
			pattern_idx = int.from_bytes(module_file.read(1), byteorder='big', signed=False)
			song_pos.append(pattern_idx)
			max_pattern_idx = max(max_pattern_idx, pattern_idx)

		mod_signature = module_file.read(4).decode('ascii')

		# read pattern data
		pattern_data = []
		for i in range(max_pattern_idx + 1):
			pattern_blob = module_file.read(1024)
			pattern_data.append(pattern_blob)

		print("song_length = " + str(song_length))
		print("old_tracker_len_info = " + str(old_tracker_len_info))
		song_pos_str = "song_pos = ["
		for i in range(song_length):
			song_pos_str += str(song_pos[i]) + ", "
		song_pos_str += "]"
		print(song_pos_str)
		print("max pattern index = " + str(max_pattern_idx))
		print(mod_signature)
		print("pattern_data = ")
		# print(pattern_data)

		file_song_end_pos = module_file.tell()
		print("end of song data at : " + str(file_song_end_pos))

		# read sample data
		print("-------------")
		print(samples)
		print("read samples binary data")

		song_tmp_path = os.path.join(folder_tmp, song_name).replace('\0', '')
		if os.path.exists(song_tmp_path):
			shutil.rmtree(song_tmp_path)
		if os.path.exists(folder_tmp):
			shutil.rmtree(folder_tmp)
		os.mkdir(folder_tmp)
		os.mkdir(song_tmp_path)

		for idx, sample in enumerate(samples):
			# iterate & export all samples
			if "sample_len" in sample:
				# read sample
				sample_data = bytearray(module_file.read(sample["sample_len"]))
				# Generate a HASH for this sample.
				# Two identical samples with share the same HASH,
				# hence the same filename
				# The HASH is generated using the old CRC16 technique,
				# whose output is shorter than MD5 or SHA1.
				h = hex_crc16(sample_data)[2:]
				# h = hashlib.md5(sample_data).hexdigest()[:24]
				samples[idx]["hash"] = h

				# sanitize the name of the sample (if any)
				sample_filename = h + '_' + (sample["sample_name"]).replace(':', '_') + '.spl'
				keepcharacters = (' ', '.', '_')
				sample_filename = "".join(c for c in sample_filename if c.isalnum() or c in keepcharacters).rstrip().replace(" ", "")
				if idx < 10:
					sample_filename = '0' + str(idx) + '_' + sample_filename
				else:
					sample_filename = str(idx) + '_' + sample_filename

				packed_sample_filename = h
				#
				# with open(os.path.join(song_tmp_path, sample_filename), "wb") as outfile:
				# 	outfile.write(sample_data)

				# write_packed_file(os.path.join(song_tmp_path, sample_filename), os.path.join(out_path, packed_sample_filename))

				# "debug" wave output
				sample_unpacked = struct.unpack('b'*sample["sample_len"], sample_data)
				# sample_data_unsigned = struct.pack('B', sample_unpacked)
				wav_sample_filename = sample_filename.replace('.spl', '.wav')
				wavfile = wave.open(os.path.join(song_tmp_path, wav_sample_filename), 'wb')
				wavfile.setnchannels(1)
				wavfile.setsampwidth(1)
				wavfile.setframerate(8000)
				wavfile.setnframes(sample["sample_len"])
				for b in sample_unpacked:
					wavfile.writeframes(struct.pack('B', b + 128))
				wavfile.close()

				if DEBUG_SAMPLE_COMPRESSION: # Debug
					# OLD PACKING METHOD
					# turn 8bits wav into 16 bit wav
					result = os.popen('sox\\sox.exe ' + ' "' + os.path.join(song_tmp_path, wav_sample_filename) + '"' + ' -b 16 ' + '"' + os.path.join(song_tmp_path, '16_' + wav_sample_filename) + '"').read()

					# compress into adpcm
					result = os.popen('adpcm\\ADPCMCodec.exe encode ' + ' "' + os.path.join(song_tmp_path, '16_' + wav_sample_filename) + '"' + ' ' + '"' + os.path.join(song_tmp_path, wav_sample_filename.replace(".wav", ".adpcm")) + '"').read()

					# # uncompress the adpcm for quality check
					result = os.popen('adpcm\\ADPCMCodec.exe decode ' + ' "' + os.path.join(song_tmp_path, wav_sample_filename.replace(".wav", ".adpcm")) + '"' + ' ' + '"' + os.path.join(song_tmp_path, 'adpcm_' + wav_sample_filename) + '"').read()

					# 	# write_packed_file(os.path.join(song_tmp_path, wav_sample_filename.replace(".wav", ".adpcm")), os.path.join(song_tmp_path, packed_sample_filename + '_adpcm_pak'))
					# 	# write sample (packed format)
					# 	write_packed_file(os.path.join(song_tmp_path, wav_sample_filename.replace(".wav", ".adpcm")), os.path.join(out_path, packed_sample_filename), packer_name, header="ADPK")

				print("\nprocessing Audio sample : " + wav_sample_filename)
				cmd_line = "python.exe" + " audio_sample.py" + " --samplefile " + os.path.join(song_tmp_path, wav_sample_filename)
				cmd_line += " --destfile " + os.path.join(out_path, packed_sample_filename)
				cmd_line += " --frequency -1"
				cmd_line += ' --packer ' + packer_name
				cmd_line += ' --encoder ' + 'adpcm' # '8svx' #
				print(cmd_line)
				process = subprocess.Popen(cmd_line, stdout=subprocess.PIPE)
				for line in iter(process.stdout.readline, b''):  # replace '' with b'' for Python 3
					print(line)
					# if line is not None:
					# 	_str_line = line.decode()
					# 	if "[original sample size]:" in _str_line:
					# 		_str_line = _str_line.replace("[original sample size]:", '')
					# 		_str_line = _str_line.strip(' \r\n')
					# 		max_sfx_length = max(max_sfx_length, int(_str_line))


		# re-read the whole song data from the start, without the samples
		module_file.seek(0)
		song_data = module_file.read(file_song_end_pos)

		# ------------------------------------------------------------
		# WRITE MODULE + SAMPLE LIST ---------------------------------
		# ------------------------------------------------------------

		tmp_song_file = os.path.join(song_tmp_path, "song_data_" + song_name.replace('\0', ''))
		with open(tmp_song_file, "wb") as outfile:

			# write original song data
			outfile.write(song_data)

			# write the 32 hash'es of the samples (0000 if no sample)
			tmp_sample_list = os.path.join(song_tmp_path, "spl_list_" + song_name.replace('\0', ''))

			#	Samples hash file
			outfile.write("SMPL".encode())
			for idx, sample in enumerate(samples):
				if "hash" in sample:
					sample_hash = sample["hash"] + '\0' * (4 - len(sample["hash"]))
				else:
					sample_hash = "\0" * 4
				outfile.write(sample_hash.encode())

		final_mod_name = os.path.join(out_path, song_name.replace('\0', ''))

		if os.path.exists(final_mod_name + ".pak"):
			os.remove(final_mod_name + ".pak")

		# compress the final file, that icnludes the song data + sample list
		write_packed_file(tmp_song_file, final_mod_name, packer_name, "IMPK", total_module_size)
		# write_packed_file(tmp_sample_list, final_mod_name, packer_name, "SMPL")

		print("protracker.py : ALL DONE!")
		exit(1)

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Protracker Module converter')
	parser.add_argument('--modfile', required=True)
	parser.add_argument('--destfile', required=True)
	parser.add_argument('--packer', required=False)

	args = parser.parse_args()
	write_module_pak(args.modfile.replace('\\', '/'), args.destfile.replace('\\', '/'), args.packer)

# write_module_pak("resources/oxygene2.mod", "../game/assets/oxygene2")
# write_module_pak("resources/cnossos_2.mod", "../game/assets/cnossos_2", packer_name="miniz")
# write_module_pak("resources/cnossos.mod", "../game/assets/cnossos", packer_name="miniz")
# write_module_pak("resources/boat.mod", "../game/assets/boat", packer_name="miniz")
# write_module_pak("resources/ending.mod", "../game/assets/ending", packer_name="shrinkler")