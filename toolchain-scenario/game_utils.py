import hashlib
import os
from game_utils import *
from game_dictionnary import *
from game_config import *

def find_element_in_list(element, list_element):
	try:
		index_element = list_element.index(element)
		return index_element
	except ValueError:
		return None


def bool_to_str(flag):
	if flag:
		return 'TRUE'
	else:
		return 'FALSE'


def str_or_none(val):
	if val is None:
		return 'None'
	else:
		return str(val)


def tr(original_string):
	translated_string = original_string
	return translated_string


def obfuscate_filename(salt, filename_str):
	s = hashlib.sha1()
	s.update((filename_str.split(".")[0] + salt).encode())

	return filename_str[:3] + '_' + s.hexdigest()[::3] + "." + filename_str.split(".")[-1]


def format_vue_integer(idx):
	_str = ""
	if idx < 10:
		_str += "0"
	if idx < 100:
		_str += "0"
	_str += str(idx)
	return _str


def exit_sum_to_dir_flags(_sum):
	dir_flags = [False] * len(game_cardinal_order)
	for dir_idx, dir_val in enumerate([1, 2, 8, 4]):
		if _sum & dir_val:
			dir_flags[dir_idx] = True

	return dir_flags


# zone parser
# reads ZNE files and get the zones coordinates
def zone_parser(zone_native_file_list):
	print("zone_parser()")

	zones = {}
	empty_zone = {"rect": [0, 0, 0, 0], "variable": None, "index": 0, "vue": -1}

	zone_index = 0
	for zone_native_file in zone_native_file_list:
		with open(zone_native_file, encoding='cp437') as zone_raw:
			zone_raw_str = zone_raw.readlines()
			for zone_line in zone_raw_str:
				zone_line = zone_line.strip()
				if zone_line is not None and len(zone_line) > 0:
					mode = zone_line[0]
					if mode == '#':
						object_name = zone_line[15:]
						if len(object_name) >= 1:
							vue_index = zone_line[1:3]
							vue_index = int(vue_index)
							if vue_index > 0:
								temp_room = {"zone_list": []}
								if str(vue_index) in zones:
									temp_room = zones[str(vue_index)]

								x = int(zone_line[3:6].strip())
								y = int(zone_line[6:9].strip())
								w = int(zone_line[9:12].strip())
								h = int(zone_line[12:15].strip())
								new_zone = empty_zone.copy()
								new_zone["rect"] = [x, y, x + w, y + h]
								new_zone["variable"] = object_name
								new_zone["index"] = zone_index
								new_zone["vue"] = vue_index

								temp_room["zone_list"].append(new_zone)
								zones[str(vue_index)] = temp_room
								zone_index += 1

	zone_max = zone_index
	print("found " + str(zone_max) + " zones!")
	return zones, zone_max

# zone sample parser
# reads ZNE files and ger the samples list
def zone_sample_parser(zone_native_file_list):
	samples_per_world = {}
	samples_filename = []
	for zone_native_file in zone_native_file_list:
		samples = []
		sample_index = 0
		with open(zone_native_file, encoding='cp437') as zone_raw:
			zone_raw_str = zone_raw.readlines()
			for zone_line in zone_raw_str:
				zone_line = zone_line.strip()
				if zone_line is not None and len(zone_line) > 0:
					mode = zone_line[0]
					if mode == '*':
						sample_name = zone_line[1:]
						sample_index += 1
						# sample_vue = int(zone_line[1:3])
						samples.append({"name": sample_name, "index": sample_index}) # , "vue": sample_vue})
						samples_filename.append(sample_name)
		world_key = zone_native_file.split('/')[-1].lower().replace('.zne', '')
		world_key = replace_athanor_semantic(world_key)
		samples_per_world[world_key] = samples

	# remove duplicates
	samples_filename = list(set(samples_filename))
	samples_filename.sort() # make the list as stable as possible, accross various updates
	return samples_per_world, samples_filename

# game version parser
def parse_game_version(gfa_listing_filename):
	atari_version = 'v0.0'
	atari_version_parse_state = 0

	if os.path.exists(gfa_listing_filename):
		with open(gfa_listing_filename, encoding='cp437') as listing_raw:
			# out = open(world_file, "w")
			listing_raw_str = listing_raw.readlines()
			for listing_line in listing_raw_str:
				listing_line = listing_line.strip()
				if listing_line is not None:
					if atari_version_parse_state == 0:
					# 	if listing_line.lower().count("*") > 15:
					# 		atari_version_parse_state = 1
					# if atari_version_parse_state == 1:
					# 	if listing_line.lower().find("safargames") > -1:
					# 		atari_version_parse_state = 2
					# if atari_version_parse_state == 2:
						if listing_line.lower().replace(' ', '').startswith('ver$="v'):
							atari_version = listing_line.lower().replace(' ', '')
							atari_version = atari_version.replace('ver$="', '')
							atari_version = atari_version.replace('"', '')
							atari_version = atari_version.upper()
							print("found Atari version number : " + atari_version)
							atari_version_parse_state = 3
					if atari_version_parse_state == 3:
						break
	return atari_version


def parse_vue_from_gfa_data_line(_line, game_part_name):
	last_line = False
	is_real_vue = False
	ops = _line[5:].split(",")
	if ops[0] == "-1":
		last_line = True

	# blank block
	data_block = {"block_name": None, "data": {"vue": None, "short": None, "bitmap": None, "links":{}}}

	# name of the block (unique key/hash)
	data_block["block_name"] = vue_index_to_game_part(int(ops[0])) + format_vue_integer(int(ops[0]))

	# vue index (integer)
	data_block["data"]["vue"] = int(ops[0])

	# bitmap filename
	if -1 < int(ops[0]) < len(vue_short_names):
		data_block["data"]["short"] = vue_short_names[int(ops[0])]
	# former code used to assume that the number of the vue = the filename of the vue
	# vue_filename = "vue_"
	# if int(ops[0]) < 10:
	# 	vue_filename += "0"
	# data_block["data"]["bitmap"] = vue_filename + ops[0] + ".pak"
	vue_filename = ops[1]
	data_block["data"]["bitmap"] = vue_filename + ".pak"

	# cardinal directions (links)
	dir_mask = exit_sum_to_dir_flags(int(ops[3][0:2])) # get "vue sum" that tells, in the end, what exits are really open or not
	for idx, dir in enumerate(game_cardinal_order):
		if idx < (len(ops[3]) - 2) / 2:
			dir_idx = int(ops[3][idx * 2 + 2:idx * 2 + 4])
			if dir_idx != 0 and dir_mask[idx]:
				data_block["data"]["links"][dir] = {"room": vue_index_to_game_part(dir_idx) + format_vue_integer(dir_idx)}

	# special case (safar specs) : "99" means "end of data"
	if data_block["data"]["vue"] < 99:
		is_real_vue = True
	return data_block, last_line, is_real_vue


# create an intermediary World Json file based on the GFA basic source
def parse_vues_links(gfa_listing_filename):
	enable_world_parser = False
	game_part_name = None  # RAPA, CNOSSOS or INDUS
	parsed_world_dict = {"init": {"room": "cnossos025", "objects": {}}, "rooms": {}}

	if os.path.exists(gfa_listing_filename):
		with open(gfa_listing_filename, encoding='cp437') as listing_raw:
			# out = open(world_file, "w")
			listing_raw_str = listing_raw.readlines()
			for listing_line in listing_raw_str:
				listing_line = listing_line.strip()
				if listing_line is not None:
					if enable_world_parser:
						if listing_line.lower().endswith("_vues:"):
							game_part_name = listing_line.lower()[:-6]
							game_part_name = replace_athanor_semantic(game_part_name)
						if listing_line.lower().startswith("data"):
							new_vue, last_line, is_real_vue = parse_vue_from_gfa_data_line(listing_line, game_part_name)
							if last_line:
								enable_world_parser = False
							else:
								if is_real_vue:
									parsed_world_dict["rooms"][new_vue["block_name"]] = new_vue["data"]
							# print(new_vue)

					if listing_line.lower().startswith("vuestruct:"):
						enable_world_parser = True

			# out.write(json.dumps(parsed_world_dict, sort_keys=False, indent=4))

	return parsed_world_dict