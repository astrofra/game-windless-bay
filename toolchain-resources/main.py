#!/usr/bin/python

"""
For license, see gpl-3.0.txt
"""
import os
import uuid
import math
import shutil
import json
import ntpath
import logging
import datetime
import subprocess
import struct
import hashlib
import colorsys
from hostplatform import host_platform

aga_mode = False
# host_platforms = "osx"
folder_in = "resources/"
folder_out = "../game/"
folder_assets = os.path.join(folder_out, "assets")
folder_tmp = "_temp_/"
packed_file_ext = '.pak'
file_logging = "bitmaps_convert.log"
res_file = "gfx.res"
disk_dispatch_file = "dispatch.json"
disk_dispatch_shell_script = "dispatch.sh"
max_packed_file_size = 0
max_sfx_length = 0
max_display_color = 32

MAX_FILESIZE_PADDING = 16
FILENAME_OBFUSCATION_ENABLED = False
MAX_FILENAME_CHAR_LEN = 64
FILENAME_SALT = "CrueltyhasahumanheartAndJealousyahumanfaceTerrorthehumanformdivineAndSecresythehumandressThehumandressisforgedironThehumanformafieryforgeThehumanfaceafurnacesealedThehumanheartitshungrygorge"
DEFAULT_COMPRESSOR = "miniz" # "shrinkler" # ""miniz"
COMPRESSOR_CAPS = {"miniz":True, "shrinkler":True, "nrv2x":False}

if aga_mode:
	folder_in = folder_in.replace('/', '') + "-aga" + '/'
	max_display_color = 128


def apply_compressor_caps(compressor_name):
	if COMPRESSOR_CAPS[compressor_name]:
		return compressor_name
	else:
		return DEFAULT_COMPRESSOR


def read_file_dispatch(dispatch_filename):
	with open(dispatch_filename) as json_data:
		return json.loads(json_data.read(), encoding="UTF8")


def obfuscate_filename(filename_str):
	s = hashlib.sha1()
	s.update((filename_str.split(".")[0] + FILENAME_SALT).encode())

	return filename_str[:3] + '_' + s.hexdigest()[::3] + "." + filename_str.split(".")[-1]


def make_temp_folder():
	try:
		shutil.rmtree(folder_tmp)
	except:
		logging.warning("cannot remove : " + folder_tmp)

	try:
		os.makedirs(folder_tmp, exist_ok=True)
	except:
		logging.warning("cannot create : " + folder_tmp)


def chunks(l, n):
	for i in range(0, len(l), n):
		yield l[i:i + n]


def pack_list_with(packer_name, planes):

	data_list = []
	for plane in planes:
		for word in plane:
			data_list.append(word)

	if packer_name is not None:
		tmp_filename = str(uuid.uuid4())
		print("Using '" + packer_name + "' compressor.")
		print("Temp filename = '" + tmp_filename + "'.")
		print("unpacked size  = " + str(len(data_list) * 2))
		srcfile = os.path.join(folder_tmp, tmp_filename + '.data')
		destfile = os.path.join(folder_tmp, tmp_filename + '.pak')
		if os.path.exists(destfile):
			os.remove(destfile)

		with open(srcfile, 'wb') as outfile:
			for val in data_list:
				outfile.write(val.to_bytes(2, byteorder='big', signed=False))

		if packer_name == "shrinkler":
			if host_platform() == 'win':
				result = os.popen('packer_shrinkler45\\win\\Shrinkler.exe -d -i 9' + ' "' + srcfile + '"' + ' ' + ' "' + destfile + '"').read()
			if host_platform() == 'osx':
				result = os.popen('packer_shrinkler45/osx/Shrinkler -d -i 9' + ' "' + srcfile + '"' + ' ' + ' "' + destfile + '"').read()
		elif packer_name == "miniz":
			if host_platform() == 'win':
				result = os.popen('packer_miniz\\win\\miniz.exe -l10 c' + ' "' + srcfile + '"' + ' ' + ' "' + destfile + '"').read()
			if host_platform() == 'osx':
				result = os.popen('packer_miniz/osx/miniz -l10 c' + ' "' + srcfile + '"' + ' ' + ' "' + destfile + '"').read()
		elif packer_name == 'nrv2x':
			if host_platform() == 'win':
				result = os.popen('packer_nrv2x\\nrv2x.exe -es -k' + ' -o "' + destfile + '"' + ' ' + '"' + srcfile + '"').read()
			if host_platform() == 'osx':
				raise NameError('nrv2x not found on OSX!')
		if not os.path.exists(destfile):
			print("/!\\ File '" + destfile + "' was not created, cannot pack file '" + srcfile)
			destfile = srcfile

		pack_data_list = []
		with open(destfile, "rb") as infile:
			while True:
				b = infile.read(1)
				if b:
					b = struct.unpack('b', b)[0]
					pack_data_list.append(b)
				else:
					break

			data_list = pack_data_list
	print("packed size = " + str(len(data_list)))
	return data_list

def color_to_plane_bits(color, depth):
	"""returns the bits for a given pixel in a list, lowest to highest plane"""
	result = [0] * depth
	for bit in range(depth):
		if color & (1 << bit) != 0:
			result[bit] = 1
	return result


def color_to_RGB4(rgb_color_triplet):
	final_color = 0
	final_color += rgb_color_triplet[2] // 16
	final_color += ((rgb_color_triplet[1] // 16) << 4)
	final_color += ((rgb_color_triplet[0] // 16) << 8)
	return final_color


def color_to_RGB8(rgb_color_triplet):
	final_color = 0
	final_color += rgb_color_triplet[2]
	final_color += ((rgb_color_triplet[1]) << 8)
	final_color += ((rgb_color_triplet[0]) << 16)
	return final_color


def color_table_to_depth(colors=[]):
	colors_amount = 2 * int(len(colors) / 2)
	if colors_amount < len(colors):
		colors_amount = len(colors) + 1
	depth = int(math.log(colors_amount, 2))
	return depth


# Mode : 'C', 'RAW'
def write_amiga_image(imdata, colors, width, height, destfile, compression=None, interleaved=False, aga_mode=False):
	# destfile = os.path.splitext(destfile)[0]

	# # imdata = im.getdata()
	# # width, height = im.size
	# # colors = [i for i in chunks(map(ord, im.palette.tostring()), 3)]
	# colors_amount = 2 * int(len(colors) / 2)
	# if colors_amount < len(colors):
	# 	colors_amount = len(colors) + 1
	# depth = int(math.log(colors_amount, 2))
	depth = color_table_to_depth(colors)
	print("name, width, height, colors, depth = %s, %d, %d, %d, %d." % (destfile, width, height, len(colors), depth))

	map_words_per_row = width // 16
	if width % 16 > 0:
		map_words_per_row += 1

	# create the converted planar data
	planes = [[0] * (map_words_per_row * height) for _ in range(depth)]
	for y in range(height):
		x = 0
		while x < width:
			# build a word for each plane
			for i in range(min(16, width - x)):
				# get the palette index for pixel (x + i, y)
				color = imdata[y * width + x + i]  # color index
				planebits = color_to_plane_bits(color, depth)
				# now we need to "or" the bits into the words in their respective planes
				wordidx = (x + i) / 16  # word number in current row
				pos = int(y * map_words_per_row + wordidx)  # list index in the plane
				for planeidx in range(depth):
					if planebits[planeidx]:
						planes[planeidx][pos] |= (1 << (15 - (x + i) % 16)) # 1 << ((x + i) % 16)
			x += 16

	##  Header file
	image_name = ntpath.basename(destfile)

	filesize = 0

	with open(destfile + '.pak', 'wb') as c_outfile:
		#	File Header
		c_outfile.write('IMPK'.encode())
		#	Image specs (width, height, depth)
		c_outfile.write(width.to_bytes(2, byteorder='big', signed=False))
		c_outfile.write(height.to_bytes(2, byteorder='big', signed=False))
		c_outfile.write(depth.to_bytes(2, byteorder='big', signed=False))
		# c_outfile.write('\0'.encode())

		# Palettes
		if aga_mode:
			c_outfile.write('PAL8'.encode())
			for color in colors:
				c_outfile.write(color_to_RGB8(color['color']).to_bytes(4, byteorder='big', signed=False))
		else:
			c_outfile.write('PAL4'.encode())
			for color in colors:  ## chunks(map(ord, im.palette.tostring()), 3):
				c_outfile.write(color_to_RGB4(color['color']).to_bytes(2, byteorder='big', signed=False))

		# Image data
		if compression is None: # not compressed
			c_outfile.write('DATA'.encode())
			for plane in planes:
				for chunk in chunks(plane, map_words_per_row):
					for word in chunk:
						c_outfile.write(word.to_bytes(2, byteorder='big', signed=False))

		else: # compressed
			if compression == "shrinkler":
				c_outfile.write('SHRK'.encode())
			elif compression == "miniz":
				c_outfile.write('MINZ'.encode())
			elif compression == 'nrv2x':
				c_outfile.write('NRV2'.encode())

			# iterate on all planes
			packed_planes = pack_list_with(compression, planes)

			print("'SIZE':")
			c_outfile.write('SIZE'.encode())

			filesize = len(packed_planes)
			print(str(filesize))
			c_outfile.write(filesize.to_bytes(2, byteorder='big', signed=False))

			for _byte in packed_planes:
				c_outfile.write(_byte.to_bytes(1, byteorder='big', signed=True))

	if FILENAME_OBFUSCATION_ENABLED:
		shutil.copy(destfile + '.pak', os.path.join(folder_assets, obfuscate_filename(image_name + '.pak')))
	else:
		shutil.copy(destfile + '.pak', os.path.join(folder_assets, image_name + '.pak'))

	return {'depth': depth, 'filesize': filesize}


def bytes_from_file(filename, chunk_size=8192):
	with open(filename, "rb") as f:
		while True:
			chunk = f.read(chunk_size)
			if chunk:
				for b in chunk:
					yield b
			else:
				break


def lum(col):
	# Todo : sort by color occurence
	r = col['color'][0]
	g = col['color'][1]
	b = col['color'][2]

	# Cosmigo ProMotion "perceived brightness"
	return math.sqrt(0.241 * r * r + 0.691 * g * g + 0.068 * b * b)

	# hls = colorsys.rgb_to_hls(r / 255.0, g / 255.0, b / 255.0)
	# return hls[1]

	# hls = colorsys.rgb_to_hsv(r / 255.0, g / 255.0, b / 255.0)
	# return hls[2]

	# Biased luma formula to "emulate" the brightness color sort of Cosmigo Promotion
	g_bias = 0.1
	return ((0.299 - (g_bias * 0.25)) * r + (0.587 + g_bias) * g + (0.114 - (g_bias * 0.75)) * b) # (Color contrast)

	# return (0.299 * r + 0.587 * g + 0.114 * b) # (Color contrast)
	# return math.sqrt(0.299 * r*r + 0.587 * g*g + 0.114 * b*b) # (HSP Color Model)
	# return math.sqrt(0.241 * r + 0.691 * g + 0.068 * b) # Luminance relative)
	# return (0.2126 * r) + (0.7152 * g) + (0.0722 * b)
	# (0.21 × R) + (0.72 × G) + (0.07 × B)

def occurence(col):
	return col['occurence']


def parse_json_palette(_palette):
	# _palette = image_json['image']['colormap']
	palette = []
	for col in _palette:
		_r = int('0x' + col[1:3], 16)
		_g = int('0x' + col[3:5], 16)
		_b = int('0x' + col[5:7], 16)
		_a = 255

		col_rgb4 = '0x' + (str(hex(_r // 16)) + str(hex(_g // 16)) + str(hex(_b // 16))).replace('0x', '')

		palette.append({'color_hex': col, 'color': [_r, _g, _b, _a], 'color_hex_rgb4': col_rgb4, 'occurence':0, 'col_str': str(_r) + str(_g) + str(_b)})

	return palette


def color_to_depth(colors):
	i = 1
	while (1 << i) < colors:
		i += 1

	return i


def convert_png_to_palette_file(pngfile, asset_name=None, palette_size=None, data_processor=None):
	make_temp_folder()

	logging.info(pngfile)
	filename = ntpath.basename(pngfile)
	if asset_name is None:
		asset_name = filename.split('.')[0]

	# Extract image meta data using Image Magick
	if host_platform() == 'win':
		image_convert_extract = os.popen('imagemagick\\win\\convert.exe' + ' "' + pngfile + '"' + ' json:').read()
	if host_platform() == 'osx':
		image_convert_extract = os.popen('convert' + ' "' + pngfile + '"' + ' json:').read()
	image_json = json.loads(image_convert_extract)
	if type(image_json) is list:
		image_json = image_json[0]

	colors = parse_json_palette(image_json['image']['colormap'])
	if palette_size is None:
		depth = color_table_to_depth(colors)
		palette_size = 1 << depth
	else:
		depth = color_to_depth(palette_size)

	destfile = os.path.join(folder_tmp, asset_name)
	image_name = ntpath.basename(destfile)

	if data_processor is None or ((data_processor is not None) and (data_processor.lower() != "no_color_swap")):
		if palette_size > 15:  # len(palette) > 15:
			if aga_mode:
				light_col_idx = int((max_display_color * 31) / 32)
				dark_col_idx = int((max_display_color * 4) / 32)
				colors[19], colors[light_col_idx] = colors[light_col_idx], colors[19]
				colors[18], colors[dark_col_idx] = colors[dark_col_idx], colors[18]
			else:
				colors[19], colors[31] = colors[31], colors[19]
				colors[18], colors[4] = colors[4], colors[18]

	with open(destfile + '.pak', 'wb') as c_outfile:
		#	File Header
		c_outfile.write('PALT'.encode())
		c_outfile.write(depth.to_bytes(2, byteorder='big', signed=False))
		# c_outfile.write('\0'.encode())

		# Palettes
		if aga_mode:
			c_outfile.write('PAL8'.encode())
			for color in colors[:palette_size]:
				c_outfile.write(color_to_RGB8(color['color']).to_bytes(4, byteorder='big', signed=False))
		else:
			c_outfile.write('PAL4'.encode())
			for color in colors[:palette_size]:  ## chunks(map(ord, im.palette.tostring()), 3):
				c_outfile.write(color_to_RGB4(color['color']).to_bytes(2, byteorder='big', signed=False))


	shutil.copy(destfile + '.pak', os.path.join(folder_assets, image_name + '.pak'))


def convert_png_to_packed_file(pngfile, asset_name=None, file_compressor=DEFAULT_COMPRESSOR, data_processor=None, auto_make_mask=False, interleaved_bitmap=False):
	make_temp_folder()

	logging.info(pngfile)
	filename = ntpath.basename(pngfile)
	if asset_name is None:
		asset_name = filename.split('.')[0]

	# Extract image meta data using Image Magick
	if host_platform() == 'win':
		image_convert_extract = os.popen('imagemagick\\win\\convert.exe' + ' "' + pngfile + '"' + ' json:').read()
	if host_platform() == 'osx':
		image_convert_extract = os.popen('convert' + ' "' + pngfile + '"' + ' json:').read()
	image_json = json.loads(image_convert_extract)
	if type(image_json) is list:
		image_json = image_json[0]

	try:
		depth = int(str(image_json['image']['depth']))
		if depth > 8 or image_json['image']['type'] == 'TrueColor':
			logging.warning("!not an indexed palette image!")
			return None
	except:
		logging.warning("!error when checking image depth!")
		return None

	try:
		if str(image_json['image']['mimeType']).lower() != 'image/png':
			logging.warning("!not a PNG file!")
			return None
	except:
		logging.warning("!error when checking image format!")
		return None

	width, height = int(image_json['image']['geometry']['width']), int(image_json['image']['geometry']['height'])
	logging.info("width, height = " + str(width) + ", " + str(height))

	# Extract image as RGBA using Image Magick
	_s = pngfile
	file_rgba = os.path.join(folder_tmp, filename.split('.')[0]) + '.rgba'
	if host_platform() == 'win':
		cmd_line = 'imagemagick\\win\\convert.exe' + ' "' + _s + '"' + ' -depth 8' + ' "' + file_rgba + '"'
	if host_platform() == 'osx':
		cmd_line = 'convert' + ' "' + _s + '"' + ' -depth 8' + ' "' + file_rgba + '"'
	process = subprocess.Popen(cmd_line, stdout=subprocess.PIPE, shell=True)
	for line in iter(process.stdout.readline, b''):  # replace '' with b'' for Python 3
		print(line)

	if os.path.exists(file_rgba):
		logging.info("File converted")
		for line in iter(process.stdout.readline, b''):
			logging.info(line)
	else:
		logging.warning("!something went wrong!")
		for line in iter(process.stdout.readline, b''):
			logging.warning(line)
		return None

	# result = True
	# retry_count = 0
	# while result and retry_count < 5:
	# 	result = subprocess.call(cmd_line, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	# 	if not result:
	# 		logging.info("File converted")
	# 		break
	# 	else:
	# 		logging.warning("!something went wrong!")
	# 		retry_count += 1
	# 		logging.warning("Retrying " + str(retry_count) + " time(s).")

	# if result:
	# 	return None
	# logging.info(result.stderr.read())

	del _s

	if not auto_make_mask:
		# Extract palette
		# _palette = image_json['image']['colormap']
		# palette = []
		# for col in _palette:
		# 	_r = int('0x' + col[1:3], 16)
		# 	_g = int('0x' + col[3:5], 16)
		# 	_b = int('0x' + col[5:7], 16)
		# 	_a = 255
		#
		# 	col_rgb4 = '0x' + (str(hex(_r // 16)) + str(hex(_g // 16)) + str(hex(_b // 16))).replace('0x', '')
		#
		# 	palette.append({'color_hex': col, 'color': [_r, _g, _b, _a], 'color_hex_rgb4': col_rgb4, 'occurence':0, 'col_str': str(_r) + str(_g) + str(_b)})
		#
		# del _palette

		palette = parse_json_palette(image_json['image']['colormap'])

		# Truncate the palette
		# Count colors
		_distinct_colors = []
		pixel = []
		for b in bytes_from_file(file_rgba):
			pixel.append(int(b))
			if len(pixel) >= 4:
				pixel = str(pixel[0]) + str(pixel[1]) + str(pixel[2])
				for _col_idx in range(len(palette)):
					if palette[_col_idx]['col_str'] == pixel:
						palette[_col_idx]['occurence'] += 1

				if not (pixel in _distinct_colors):
					_distinct_colors.append(pixel)
				pixel = []

		d = int(math.log(len(_distinct_colors), 2))
		while pow(2, d) < len(_distinct_colors):
			d += 1

		pal_len = pow(2, d)
		palette = palette[0:pal_len]

		print("Counted " + str(pal_len) + " colors.")

		# Sort the palette by luminance
		# palette.sort(key=lum)
		# palette.sort(key=lum, reverse=True)
		# palette.sort(key=occurence, reverse=True)

		# Organize color by luma so that the UI & mouse look OK
		# /!\ The routine may crash here if the image doesn't use
		# every color of the palette.
		# Ex : the image is meant to be in 32 colors but the image
		# only uses 16 colors.
		# Workaround : put some tiny gradient in the image using
		# each one of the 32 colors.
		actual_max_col = len(palette)

		if actual_max_col < max_display_color: # fixme!
			for i in range(max_display_color - actual_max_col):
				palette.append({'color_hex': 0xFFFFFF, 'color': [255,255,255,255], 'color_hex_rgb4': 0xFFF, 'occurence':0, 'col_str': "FFF"})

		# swap 4 colors, according to a hardware constraint
		if data_processor is None or ((data_processor is not None) and (data_processor.lower() != "no_color_swap")):
			if actual_max_col > 15:  # len(palette) > 15:
				if aga_mode:
					light_col_idx = int((max_display_color * 31) / 32)
					dark_col_idx = int((max_display_color * 4) / 32)
					palette[19], palette[light_col_idx] = palette[light_col_idx], palette[19]
					palette[18], palette[dark_col_idx] = palette[dark_col_idx], palette[18]
				else:
					palette[19], palette[31] = palette[31], palette[19]
					palette[18], palette[4] = palette[4], palette[18]

		# Reindex each pixel according the order of the palette
		def find_color(scol = [0,0,0,255]):
			col_idx = 0
			for dcol in palette:
				if dcol['color'] == scol:
					return col_idx
				col_idx += 1

			logging.warning("!find_color() could not find the color index!")
			return -1

		indexed_bitmap = []
		pixel = []
		max_col_index = 0
		for b in bytes_from_file(file_rgba):
			pixel.append(int(b))
			if len(pixel) >= 4:
				_col_idx = find_color(pixel)
				if _col_idx >= 0:
					indexed_bitmap.append(_col_idx)
					max_col_index = max(max_col_index, _col_idx)
				pixel = []

		palette = palette[0:max_col_index + 1]

		if len(palette) < max_display_color: # fixme!
			for i in range(max_display_color - len(palette)):
				palette.append({'color_hex': 0xFFFFFF, 'color': [255,255,255,255], 'color_hex_rgb4': 0xFFF, 'occurence':0, 'col_str': "FFF"})

		# logging.warning("Max color index : " + str(max_col_index))

	elif auto_make_mask:
		indexed_bitmap = []
		pixel = []
		for b in bytes_from_file(file_rgba):
			pixel.append(int(b))
			if len(pixel) >= 4:
				_col = pixel[0] + pixel[1] + pixel[2]
				if _col > 0:
					indexed_bitmap.append(1)
				else:
					indexed_bitmap.append(0)
				pixel = []

		palette = []
		palette.append({'color_hex': 0x000, 'color': [0, 0, 0, 0], 'color_hex_rgb4': 0x000})
		palette.append({'color_hex': 0xFFFFFF, 'color': [0xFF, 0xFF, 0xFF, 0xFF], 'color_hex_rgb4': 0xFFF})

		max_col_index = 1

	# Write Amiga image as a C file
	# compressor_name = 'miniz'
	# # compressor_name = 'shrinkler'
	# # compressor_name = 'lz4'
	# # compressor_name = 'osdk'
	# # compressor_name = 'exomizer'
	result = write_amiga_image(indexed_bitmap, palette, width, height, os.path.join(folder_tmp, asset_name), compression=file_compressor, interleaved=interleaved_bitmap, aga_mode=aga_mode)

	# Keep only the Amiga palette
	rgb4_pal = []
	for col in palette:
		rgb4_pal.append(col['color_hex_rgb4'])

	img_dict = {'width': width, 'height': height, 'depth': result['depth'], 'palette_size': len(palette), 'palette': rgb4_pal, 'filesize':result['filesize']}

	return img_dict


if __name__ == '__main__':
	logging.basicConfig(filename=file_logging, level=logging.DEBUG)

	logging.info("------------------------------\nNew batch @ " + str(datetime.datetime.now()))

	make_temp_folder()

	# palette_list = {}
	# bitmap_list = {}
	# sprite_list = {}
	# module_list = {}
	# sample_list = {}
	# text_list = {}
	max_packed_file_size = 0
	max_sfx_length = 0

	# fetch the .res file
	# this file shall contain some specific instructions
	res_list = []
	res_file_path = os.path.join(folder_in, res_file)
	with open(res_file_path) as dialog_raw:
		dialog_raw_str = dialog_raw.readlines()
		for res_line in dialog_raw_str:
			res_line = res_line.strip()
			if res_line is not None:
				res_words = res_line.lower().split(' ')
				if res_words[0] == "palette":
					# extract palette
					new_res_entry = {'asset_type': res_words[0], 'asset_name': None, 'file_name': None, 'palette_size': None, 'data_processor': None}
					if len(res_words) > 1:
						new_res_entry["asset_name"] = res_words[1]
					if len(res_words) > 2:
						new_res_entry["file_name"] = res_words[2].replace('"', '')
					if len(res_words) > 3:
						new_res_entry["palette_size"] = int(res_words[3].replace('"', ''))
					if len(res_words) > 4:
						new_res_entry["data_processor"] = res_words[4]
					if "file_name" in new_res_entry:
						# palette_list[new_res_entry["file_name"]] = new_res_entry
						res_list.append(new_res_entry)
				if res_words[0] == "image":
					new_res_entry = {'asset_type': res_words[0], 'asset_name': None, 'file_name': None, 'file_compressor': DEFAULT_COMPRESSOR, 'data_processor': None}
					if len(res_words) > 1:
						new_res_entry["asset_name"] = res_words[1]
					if len(res_words) > 2:
						new_res_entry["file_name"] = res_words[2].replace('"', '')
					if len(res_words) > 3:
						new_res_entry["file_compressor"] = apply_compressor_caps(res_words[3])
					if len(res_words) > 4:
						new_res_entry["data_processor"] = res_words[4]
					if "file_name" in new_res_entry:
						# res_list[new_res_entry["file_name"]] = new_res_entry
						res_list.append(new_res_entry)
				if res_words[0] == "mask":
					new_res_entry = {'asset_type': res_words[0], 'asset_name': None, 'file_name': None, 'file_compressor': DEFAULT_COMPRESSOR, 'data_processor': None}
					if len(res_words) > 1:
						new_res_entry["asset_name"] = res_words[1]
					if len(res_words) > 2:
						new_res_entry["file_name"] = res_words[2].replace('"', '')
					if len(res_words) > 3:
						new_res_entry["file_compressor"] = apply_compressor_caps(res_words[3])
					if len(res_words) > 4:
						new_res_entry["data_processor"] = res_words[4]
					if "file_name" in new_res_entry:
						# res_list[new_res_entry["file_name"]] = new_res_entry
						res_list.append(new_res_entry)
				if res_words[0] == "sprite":
					new_res_entry = {'asset_type': res_words[0], 'asset_name': None, 'file_name': None, 'sprite_height': 16}
					if len(res_words) > 1:
						new_res_entry["asset_name"] = res_words[1]
					if len(res_words) > 2:
						new_res_entry["file_name"] = res_words[2].replace('"', '')
					if len(res_words) > 3:
						new_res_entry["sprite_height"] = int(res_words[3])
					if "file_name" in new_res_entry:
						# sprite_list[new_res_entry["file_name"]] = new_res_entry
						res_list.append(new_res_entry)
				if res_words[0] == "module":
					new_res_entry = {'asset_type': res_words[0], 'asset_name': None, 'file_name': None, 'file_compressor': DEFAULT_COMPRESSOR, 'data_processor': None}
					if len(res_words) > 1:
						new_res_entry["asset_name"] = res_words[1]
					if len(res_words) > 2:
						new_res_entry["file_name"] = res_words[2].replace('"', '')
					if len(res_words) > 3:
						new_res_entry["file_compressor"] = apply_compressor_caps(res_words[3])
					if "file_name" in new_res_entry:
						# module_list[new_res_entry["file_name"]] = new_res_entry
						res_list.append(new_res_entry)
				if res_words[0] == "sample":
					new_res_entry = {'asset_type': res_words[0], 'asset_name': None, 'file_name': None, 'frequency': None, 'file_compressor': DEFAULT_COMPRESSOR, 'data_processor': None}
					if len(res_words) > 1:
						new_res_entry["asset_name"] = res_words[1]
					if len(res_words) > 2:
						new_res_entry["file_name"] = res_words[2].replace('"', '')
					if len(res_words) > 3:
						new_res_entry["file_compressor"] = apply_compressor_caps(res_words[3])
					if len(res_words) > 4:
						new_res_entry["data_processor"] = res_words[4]
					if len(res_words) > 5:
						new_res_entry["frequency"] = res_words[5]
					if "file_name" in new_res_entry:
						# sample_list[new_res_entry["file_name"]] = new_res_entry
						res_list.append(new_res_entry)
				if res_words[0] == "text":
					new_res_entry = {'asset_type': res_words[0], 'asset_name': None, 'file_name': None, 'file_compressor': DEFAULT_COMPRESSOR}
					if len(res_words) > 1:
						new_res_entry["asset_name"] = res_words[1]
					if len(res_words) > 2:
						new_res_entry["file_name"] = res_words[2].replace('"', '')
					if len(res_words) > 3:
						new_res_entry["file_compressor"] = apply_compressor_caps(res_words[3])
					if "file_name" in new_res_entry:
						# text_list[new_res_entry["file_name"]] = new_res_entry
						res_list.append(new_res_entry)

	# iterate on the PNG files (PNG so far...)
	# for file in os.listdir(folder_in):
	for file_idx, file_entry in enumerate(res_list):
		file = file_entry['file_name']
		k = res_list[file_idx]
		print("--------- packing " + file)
		# if file.find("logo_safar_games") > -1:
		# 	print("!!!!")

		pngfile = os.path.join(folder_in, file)
		# Is this a bitmap file ?
		if os.path.isfile(pngfile):
			# bitmap
			if k['asset_type'] == 'sprite':
				print("processing hardware sprite: " + file)
				if host_platform() == 'win':
					cmd_line = "python.exe" + " png2sprites.py" + " --pngfile resources/" + file + " --destfile " + k["asset_name"] + " --height " + str(k["sprite_height"])
				if host_platform() == 'osx':
					cmd_line = 'python3' + ' ' + 'png2sprites.py' + ' --pngfile "resources/' + file + '" --destfile "' + k["asset_name"] + '" --height ' + str(k["sprite_height"])
				print(cmd_line)
				result = subprocess.call(cmd_line, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=os.getcwd(), shell=True)
				print("subprocess result = " + str(result))
				# else:
				# 	destfile = os.path.join(folder_out, file.split('.')[0] + packed_file_ext)
				#
				# 	if file in res_list:
				# 		k = res_list[file]
			elif k['asset_type'] == 'image':
				result = convert_png_to_packed_file(pngfile, asset_name=k['asset_name'],
													file_compressor=k['file_compressor'],
													data_processor=k['data_processor'])
				if result is not None:
					max_packed_file_size = max(max_packed_file_size, result['filesize'])
					logging.info("processed : " + pngfile)
				else:
					logging.warning("failed : " + pngfile)
			elif k['asset_type'] == 'mask':
				result = convert_png_to_packed_file(pngfile, asset_name=k['asset_name'],
													file_compressor=k['file_compressor'],
													data_processor=k['data_processor'],
													auto_make_mask=True)
				if result is not None:
					max_packed_file_size = max(max_packed_file_size, result['filesize'])
					logging.info("processed : " + pngfile)
				else:
					logging.warning("failed : " + pngfile)
			elif k['asset_type'] == 'palette':
				convert_png_to_palette_file(pngfile, asset_name=k['asset_name'],
											 palette_size=k['palette_size'],
											 data_processor=k['data_processor'])
					# else:
					# 	result = convert_png_to_packed_file(pngfile)

					# im = Image.open(pngfile)
					# write_amiga_image(im, destfile)
					# if result is not None:
					# 	bitmap_list[file.split('.')[0]] = result
					# 	# if file.startswith('vue_'):
					# 	max_packed_file_size = max(max_packed_file_size, result['filesize'])
					# 	logging.info("processed : " + pngfile)
					# else:
					# 	logging.warning("failed : " + pngfile)
			# Protracker module
			# elif file.lower().find('.mod') > -1:
			# 	if file in module_list:
			elif k['asset_type'] == 'module':
				print("processing Protracker module : " + file)
				dest_packed_file = os.path.join(folder_assets, k["asset_name"])
				if host_platform() == 'win':
					cmd_line = "python.exe" + " protracker_module.py" + " --modfile resources/" + file + " --destfile " + dest_packed_file
				if host_platform() == 'osx':
					cmd_line = "python3" + " protracker_module.py" + " --modfile resources/" + file + " --destfile " + dest_packed_file
				cmd_line += " --packer " + k["file_compressor"]
				# cmd_line = "python.exe" + " protracker.py" + " --modfile resources/" + file + " --destfile " + os.path.join(folder_assets, module_list[file]["asset_name"]).replace('\\', '/')
				print(cmd_line)
				# result = subprocess.call(cmd_line, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=os.getcwd())
				# print("subprocess result = " + str(result))
				# with open('protracker.log', 'wb') as flog:  # replace 'w' with 'wb' for Python 3
				process = subprocess.Popen(cmd_line, stdout=subprocess.PIPE, shell=True)
				for line in iter(process.stdout.readline, b''):  # replace '' with b'' for Python 3
					print(line)
					# sys.stdout.write(line)
					# flog.write(line)
				if os.path.exists(dest_packed_file + ".pak"):
					max_packed_file_size = max(max_packed_file_size, os.stat(dest_packed_file + ".pak").st_size)
			# Audio sample
			# elif file.lower().find('.wav') > -1:
			elif k['asset_type'] == 'sample':
				# if file in sample_list:
				print("processing Audio sample : " + file)
				if host_platform() == 'win':
					cmd_line = "python.exe" + " audio_sample.py" + " --samplefile resources/" + file + " --destfile " + os.path.join(folder_assets, k["asset_name"])
				if host_platform() == 'osx':
					cmd_line = "python3" + " audio_sample.py" + " --samplefile resources/" + file + " --destfile " + os.path.join(folder_assets, k["asset_name"])
				if k["frequency"] is not None:
					cmd_line += ' --frequency ' + str(k["frequency"])
				if k["file_compressor"] is not None:
					cmd_line += ' --packer ' + k["file_compressor"]
					if k["data_processor"] is not None:
						cmd_line += ' --encoder ' + k["data_processor"]
				print(cmd_line)
				process = subprocess.Popen(cmd_line, stdout=subprocess.PIPE, shell=True)
				for line in iter(process.stdout.readline, b''):  # replace '' with b'' for Python 3
					print(line)
					if line is not None:
						_str_line = line.decode()
						if "[original sample size]:" in _str_line:
							_str_line = _str_line.replace("[original sample size]:", '')
							_str_line = _str_line.strip(' \r\n')
							max_sfx_length = max(max_sfx_length, int(_str_line))
			elif file.lower().find('.bintext') > -1:
				# if file in text_list:
				print('text compactor ------------ ' + file)
				packer_name = k['file_compressor']
				srcfile = os.path.join('resources', file)
				tmpfile = os.path.join(folder_tmp, k["asset_name"]) + '.tmp'
				if os.path.exists(tmpfile):
					os.remove(tmpfile)
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
						result = os.popen('packer_nrv2x\\nrv2x.exe -es -k' + ' -o "' + tmpfile + '"' + ' ' + '"' + srcfile + '"').read()
					if host_platform() == 'osx':
						raise NameError('nrv2x not found on OSX!')

				unpacked_file_size = os.path.getsize(srcfile)
				packed_file_size = os.path.getsize(tmpfile)
				max_packed_file_size = max(max_packed_file_size, packed_file_size)

				with open(tmpfile, 'rb') as infile:
					packed_data = infile.read(packed_file_size)

					dest_packed_file = os.path.join(folder_assets, k["asset_name"]) + '.pak'
				with open(dest_packed_file, 'wb') as outfile:
					outfile.write('TXPK'.encode())
					outfile.write(unpacked_file_size.to_bytes(2, byteorder='big', signed=False))
					if packer_name == "shrinkler":
						outfile.write('SHRK'.encode())
					elif packer_name == "miniz":
						outfile.write('MINZ'.encode())
					elif packer_name == 'nrv2x':
						outfile.write('NRV2'.encode())

					outfile.write('SIZE'.encode())
					print("packed_filesize = " + str(packed_file_size))
					outfile.write(packed_file_size.to_bytes(2, byteorder='big', signed=False))

					outfile.write(packed_data)

				if os.path.exists(dest_packed_file):
					max_packed_file_size = max(max_packed_file_size, os.stat(dest_packed_file).st_size)


	max_packed_file_size = int(max_packed_file_size * 1.05)
	max_packed_file_size_padded = ((max_packed_file_size + MAX_FILESIZE_PADDING) // MAX_FILESIZE_PADDING) * MAX_FILESIZE_PADDING
	print("max_packed_file_size = " + str(max_packed_file_size) + " -> " + str(max_packed_file_size_padded))

	max_sfx_length = int(max_sfx_length * 1.05)
	max_sfx_length_padded = ((max_sfx_length + MAX_FILESIZE_PADDING) // MAX_FILESIZE_PADDING) * MAX_FILESIZE_PADDING
	print("max_sfx_length = " + str(max_sfx_length) + " -> " + str(max_sfx_length_padded))

	with open(os.path.join(folder_out, 'assets.json'), 'w') as fp:
		json.dump(res_list, fp, sort_keys=True, indent=4)

	with open(os.path.join(folder_out, "game", "aos", 'config.h'), 'w') as fp:
		fp.write('/* Windless Bay for Amiga OCS -- (c) RESISTANCE.NO 2021 */\n')
		fp.write('/* THIS CODE WAS GENERATED BY R-PAGE RESOURCE TOOLCHAIN */\n')
		fp.write('/*                   DON\'T MODIFY IT !                  */\n\n')
		fp.write('#ifndef _FILE_CONFIG_\n')
		fp.write('#define _FILE_CONFIG_\n\n')
		# fp.write('#include "game/world.h"\n\n')
		fp.write('#define MAX_PAK_SIZE ' + str(max_packed_file_size_padded) + ' /* Maximum size (in byte) of a compressed file */\n')
		fp.write('#define MAX_SFX_SIZE ' + str(max_sfx_length_padded) + ' /* Maximum size of a 8bit mono audio sample */ \n')
		# fp.write('\n')
		# fp.write('/* file dispatch */\n')
		# fp.write('file_dispatch file_dispatch_list[' + str(len(fd)) + '] = {\n')
		# for file in fd:
		# 	fp.write('\t{ "' + file + '", ' + fd[file]["disk"] + ' },\n')
		#
		# fp.write('};\n')
		fp.write('#endif\n')
		fp.write('\n')

	with open(os.path.join(folder_out, "game", "aos", 'files.c'), 'w') as fp:
		fd = read_file_dispatch(os.path.join(folder_in, disk_dispatch_file))
		fp.write('/* Windless Bay for Amiga OCS -- (c) RESISTANCE.NO 2021 */\n')
		fp.write('/* THIS CODE WAS GENERATED BY R-PAGE RESOURCE TOOLCHAIN */\n')
		fp.write('/*                   DON\'T MODIFY IT !                  */\n\n')
		# fp.write('#include "game/world.h"\n')
		fp.write('#include "game/aos/files.h"\n\n')
		fp.write('\n')
		fp.write('/* file dispatch */\n')
		fp.write('file_dispatch file_dispatch_list[FILE_DISPATCH_LIST_SIZE] = {\n')
		for file in fd["files"]:
			fp.write('\t{ "' + file + '", ' + fd["files"][file]["disk"] + ' },\n')
		fp.write('};\n')
		fp.write('\n')

	with open(os.path.join(folder_out, "game", "aos", 'files.h'), 'w') as fp:
		fd = read_file_dispatch(os.path.join(folder_in, disk_dispatch_file))
		fp.write('/* Windless Bay for Amiga OCS -- (c) RESISTANCE.NO 2021 */\n')
		fp.write('/* THIS CODE WAS GENERATED BY R-PAGE RESOURCE TOOLCHAIN */\n')
		fp.write('/*                   DON\'T MODIFY IT !                  */\n\n')

		fp.write('#ifndef _FILES_LOCATION_\n')
		fp.write('#define _FILES_LOCATION_\n\n')
		fp.write('#include <exec/types.h>\n\n')

		fp.write('typedef struct {\n')
		fp.write('\tchar *filename;\n')
		fp.write('\tUBYTE disk;\n')
		fp.write('} file_dispatch;\n\n')

		fp.write('#define DATA_DISK_ANY 0\n')
		for idx, disk in enumerate(fd["disks"]):
			fp.write('#define DATA_DISK_' + str(idx) + ' (1 << ' + str(idx) + ') /* ' + disk + ' */\n')
		fp.write('#define DATA_DISK_MAX ' + str(len(fd["disks"])) + '\n')
		fp.write('\n')
		for idx, disk in enumerate(fd["disks"]):
			fp.write('#define DISK_NAME_' + str(idx) + ' "' + disk + '"\n')
		fp.write('\n')
		fp.write('#define FILE_DISPATCH_LIST_SIZE ' + str(len(fd["files"])) + '\n')
		fp.write('extern file_dispatch file_dispatch_list[FILE_DISPATCH_LIST_SIZE];\n')
		fp.write('\n')
		fp.write('#endif\n')
		fp.write('\n')

	with open(os.path.join(folder_out, "assets", disk_dispatch_shell_script), 'wb') as fp:
		fp.write(bytearray(';Windless Bay for Amiga OCS - (c) RESISTANCE.NO 2021\n', encoding='latin1'))
		fp.write(bytearray(';THIS SHELL SCRIPT WAS GENERATED BY R-PAGE RESOURCE TOOLCHAIN\n', encoding='latin1'))
		fp.write(bytearray(';DON\'T MODIFY IT !\n', encoding='latin1'))
		fp.write(bytearray('\n', encoding='latin1'))
		fp.write(bytearray('failat 21\n', encoding='latin1'))
		fp.write(bytearray('\n', encoding='latin1'))
		fp.write(bytearray(';create the assets folder on each disk\n\n', encoding='latin1'))
		for idx, disk in enumerate(fd["disks"]):
			fp.write(bytearray('makedir "' + disk + 'assets"\n', encoding='latin1'))
		fp.write(bytearray('\n', encoding='latin1'))
		fp.write(bytearray(';update each file\n', encoding='latin1'))
		for file in fd["files"]:
			if file != "default":
				fp.write(bytearray('\n;' + file + ' -> ' + fd["files"][file]["disk"] + '\n', encoding='latin1'))
				fp.write(bytearray('echo "updating (' + file + ')"\n', encoding='latin1'))
				disk_idx = fd["files"][file]["disk"].replace("DATA_DISK_", "")
				if disk_idx == "ANY":
					disk_idx = -1
					for disk_idx in range(len(fd["disks"])):
						fp.write(bytearray('delete "' + fd["disks"][disk_idx] + 'assets/' + file + '"\n', encoding='latin1'))
						fp.write(bytearray('copy "' + file + '" "' + fd["disks"][disk_idx] + 'assets/' + file + '"\n', encoding='latin1'))
				else:
					disk_idx = int(disk_idx)
					fp.write(bytearray('delete "' + fd["disks"][disk_idx] + 'assets/' + file + '"\n', encoding='latin1'))
					fp.write(bytearray('copy "' + file + '" "' + fd["disks"][disk_idx] + 'assets/' + file + '"\n', encoding='latin1'))

# # stress test
	# vue_amount = sum(asset.startswith('vue_') for asset in bitmap_list)
	#
	# with open(os.path.join(folder_out, "game", 'stress_test.h'), 'w') as fp:
	# 	fp.write('#ifndef _STRESS_TEST_\n')
	# 	fp.write('#define _STRESS_TEST_\n\n')
	#
	# 	fp.write('#define STRESS_TEST_SIZE ' + str(vue_amount) + '\n')
	# 	fp.write('char *stress_test[' + str(vue_amount) + '] = {\n')
	# 	for asset in bitmap_list:
	# 		if asset.startswith("vue_"):
	# 			fp.write('\t"' + asset + '",\n')
	# 	fp.write('};\n\n')
	#
	# 	fp.write('#endif\n')
	# 	fp.write('\n')

	# im = Image.open('ilkke_font.png') ##args.pngfile)
	# write_amiga_image(im, 'test2.h') ##args.destfile)

