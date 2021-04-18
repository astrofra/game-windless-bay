# text adventure parser & navigator
#
# Notes : each room or object is indexed with a unique ID,
# whose range goes from 0 to n
# This unique ID is based on an initial alphanum sort of each
# room name or object name.
# As a consequence, changing the language might change the order
# and thus the ID, hence making a save game invalid when switching
# from a language to another.
# To avoid this, the translation process occurs after the
# indexed are built.

import json
import os
from game_utils import *
from game_dictionnary import *
from game_config import *
import re
import crc8
import crc16

world_file = os.path.realpath(os.path.join(os.getcwd(), "resources/world.json"))
zones_file = os.path.realpath(os.path.join(os.getcwd(), "resources/zones.json"))
zones_samples_file = os.path.realpath(os.path.join(os.getcwd(), "resources/zones_samples.json"))

room_names = []
dialogue_count = 0
object_count = 0
music_symbols = []
sample_vue_table = {}

sprite_state_save_load = []  # store a table for the save/load routine

file_dispatch = {}
portraits = None

atari_version = None

#        INVENTAIRE

enable_world_parser = False
game_part_name = None # BATEAU, RAPA, CNOSSOS or INDUS
# parsed_world_dict = {"init": {"room": "cnossos025", "objects": {}}, "rooms": {}}

def hex_crc16(str_in):
	return hex(crc16.crc16xmodem(str_in.encode('ascii')))

def parse_inventory_object_from_gfa_data_line(_line, _part):
	is_real_object = False
	last_line = False
	# blank block
	data_block = {"object_name": None, "data": {}}

	ops = _line[5:].split(",")
	if ops[0].lower() != '"cryo"' and not (ops[0].startswith('"#')):
		object_name = ops[0].replace('"', '')
		data_block["object_name"] = object_name
		data_block["data"] = {"unidentified_data": int(ops[1]), "game_part": _part}
		is_real_object = True
	return data_block, last_line, is_real_object

inventory_objects = []

if os.path.exists(gfa_listing_file):
	with open(gfa_listing_file, encoding='cp437') as listing_raw:
		# out = open(world_file, "w")
		listing_raw_str = listing_raw.readlines()
		for listing_line in listing_raw_str:
			listing_line = listing_line.strip()
			if listing_line is not None:
				if listing_line.lower().endswith("_invent:"):
					enable_world_parser = True
					game_part_name = listing_line.lower().split("_")[0]
					game_part_name = replace_athanor_semantic(game_part_name)

				if enable_world_parser:
					if listing_line.lower().startswith("data"):
						# print(listing_line)
						new_object, last_line, is_real_object = parse_inventory_object_from_gfa_data_line(listing_line, game_part_name)
						if is_real_object:
							# parsed_world_dict["rooms"][new_vue["block_name"]] = new_vue["data"]
							print(new_object)
							inventory_objects.append(new_object)
					else:
						if listing_line.lower() == "return":
							break

# exit()

##################################
#          SPRITES
##################################


def parse_sprite_from_gfa_data_line(_line, _part):
	is_real_sprite = False
	last_line = False
	# blank block
	data_block = {"sprite_name": None, "data": {}}

	ops = _line[5:].split(",")
	if ops[0].lower() != '"cryo"' and not (ops[0].startswith('"#')):
		sprite_name = ops[0].replace('"', '')
		data_block["sprite_name"] = sprite_name
		data_block["data"] = {"vue": int(ops[1]), "frames": int(ops[2]), "enabled": abs(int(ops[3])), "game_part": _part}
		is_real_sprite = True
	return data_block, last_line, is_real_sprite

# get version number from the GFA source code

atari_version = parse_game_version(gfa_listing_file)

# get all sprites references from GFA basic source
enable_world_parser = False
game_part_name = None # BATEAU, RAPA, CNOSSOS or INDUS
# parsed_world_dict = {"init": {"room": "cnossos025", "objects": {}}, "rooms": {}}

sprites = []

if os.path.exists(gfa_listing_file):
	with open(gfa_listing_file, encoding='cp437') as listing_raw:
		out = open(world_file, "r")
		listing_raw_str = listing_raw.readlines()
		for listing_line in listing_raw_str:
			listing_line = listing_line.strip()
			if listing_line is not None:
				if listing_line.lower().endswith("_spr:"):
					enable_world_parser = True
					game_part_name = listing_line.lower()[:-5]
					game_part_name = replace_athanor_semantic(game_part_name)

				if enable_world_parser:
					if listing_line.lower().startswith("data"):
						new_sprite, last_line, is_real_sprite = parse_sprite_from_gfa_data_line(listing_line, game_part_name)
						if is_real_sprite:
							# parsed_world_dict["rooms"][new_vue["block_name"]] = new_vue["data"]
							print(new_sprite)
							sprites.append(new_sprite)
					else:
						if listing_line.lower() == "return":
							break

if os.path.exists(sprites_portraits):
	with open(sprites_portraits) as json_data:
		portraits = json.loads(json_data.read(), encoding="UTF8")

##################################
#          CLICK ZONES
##################################

# parse the ZNE zone file
# "zones" are actual click zones
#
# -- fichier .zne
# s'articule comme suit :
# ex :
# #25xxxyyylllhhhID
# *26name
#
# #25 = concerne donc la vue 25 (Le # indique que l'on a affaire à une zone de clic)
# xxx = Le X de la zone (3 chars)
# yyy = Le Y de la zone (3 chars)
# lll    = Largeur de la zone (3 chars)
# hhh= Hauteur de la zone (3 chars)
# ID  = l'ID text de la zone (16 chars) (Utiliser dans le scriptage du scénar)
#
# *26 = Concerne donc la vue 26 (Le * indique de l'on a affaire à une digit)
# name = Le filename de la digit concernée. (.spl probablement)
#
# Dans le jeu je considère des MAX à 32 zones/vue et 8 Digits/vue.
# Ce qui fait que j'ai deux tableaux à deux dimensions du style.

if True:
	zones, zone_max = zone_parser(zone_native_files)
else:
	zones, zone_max = {}, 0

zone_index = 0
for zl in zones:
	for z in range(len(zones[zl]["zone_list"])):
		zones[zl]["zone_list"][z]["index"] = zone_index
		zone_index += 1

out = open(zones_file, "w")
out.write(json.dumps(zones, sort_keys=False, indent=4))
out.close()

samples, sample_name_table = zone_sample_parser(zone_native_files)
out = open(zones_samples_file, "w")
out.write(json.dumps(samples, sort_keys=False, indent=4))
out.close()

##################################
#  SCENARIO / CONDITION SCRIPTS
##################################


def _indent(flag):
	if flag:
		return '\t'
	else:
		return ''

def process_condition_line(_line):
	# _line = _line.replace("/con", "if(")
	# get every terms in line
	new_line = ""
	terms = re.findall('\(.*?\)', _line)
	bool_operator = []

	term_idx = 0
	for term in terms:
		if term_idx < len(terms):
			_idx = _line.find(term)
			if _idx + len(term) < len(_line):
				bool_operator.append(_line[_idx + len(term)])

	term_idx = 0
	for term in terms:
		if term.find("=") > -1:
			tsplit = term.split("=")
			test_op = "=="
		elif term.find("!") > -1:
			tsplit = term.split("!")
			test_op = "!="
		elif term.find(">") > -1:
			tsplit = term.split(">")
			test_op = ">"
		elif term.find("<") > -1:
			tsplit = term.split("<")
			test_op = "<"
		l_op = tsplit[0].replace("(","")
		r_op = tsplit[1].replace(")", "")

		if r_op.isdigit():
			r_op = str(int(r_op))

		if l_op in athanor_con_dict:
			if athanor_con_dict[l_op]["type"] == "variable":
				if l_op in athanor_con_dict and athanor_con_dict[l_op]["input"] == "game_object":
					r_op = athanor_var(r_op)
				term = "(" + athanor_con_dict[l_op]["name"] + " " + test_op + " " + r_op + ")"
			elif athanor_con_dict[l_op]["type"] == "function":
				negative_term = False
				if r_op.startswith("NOT_"):
					r_op = r_op.replace("NOT_", "")
					negative_term = True
				else:
					if test_op == "!=":
						negative_term = True
				if r_op in game_action_dict:
					r_op = game_action_dict[r_op]
				elif l_op in athanor_con_dict and athanor_con_dict[l_op]["input"] == "game_object":
					r_op = athanor_var(r_op)
				term = "(" + ("", "!")[negative_term] + athanor_con_dict[l_op]["name"] + "(" + r_op + "))"
		if term_idx > 0:
			new_line = '(' + new_line + term + ')'
		else:
			new_line += term
		if term_idx < len(bool_operator):
			new_line += bool_operator[term_idx].replace("&", " && ").replace("|", " || ")
		term_idx += 1
	if len(terms) > 1:
		new_line = "(" + new_line + ")"
	new_line = "if " + new_line
	return new_line


def parse_multi_dialog(_line, dict_entry_name, _vue):
	ops = _line[_line.find("=")+1:]
	if (ops.find('-') > -1 and ops.find(',') > -1) or ops.find('-') > -1 :
		function_suffix = ''
		ops_comma = ops.replace('-', ',')
		if ops_comma.count(',') == 2:  # means 3 parameters
			function_suffix = '_ex'
		_line = _line.replace(dict_entry_name + '=', athanor_set_preprocessor_dict[dict_entry_name]["name"] + function_suffix + '=')
		_sanitized_r_op = ''
		for _idx, _sub_r_op in enumerate(ops_comma.split(',')):
			if _sub_r_op.isdigit():
				_sub_r_op = str(int(_sub_r_op))
			_sanitized_r_op += _sub_r_op
			if _idx < len(ops_comma.split(',')) - 1:
				_sanitized_r_op += ','
		ops_comma = _sanitized_r_op
		_line = _line.replace(ops, ops_comma)
	return _line


def parse_play_sample(_line, dict_entry_name, _vue):
	if _line.find("PlaySPL=") > -1:
		_spl_index = _line.replace("PlaySPL=", "")
		if _spl_index.find(','):
			_spl_index = _spl_index.split(',')[0]
		_spl_index = int(_spl_index)
		_key = str(_vue) + "_" + str(_spl_index)
		if _vue < 10:
			_key = "0" + _key
		# spl_index = # <class 'dict'>: {'name': 'PlaySPL=1', 'vue': 25}
		sample_vue_table[_key] = {"index": _spl_index, "vue": _vue}
	return _line


def preprocess_setter_line(_line, _vue):
	parse_callbacks = {
		"Dial": parse_multi_dialog,
		"PlaySPL": parse_play_sample,
	}

	newline = _line

	for dict_entry_name in athanor_set_preprocessor_dict:
		if newline.find(dict_entry_name + '=') > -1:
			newline = parse_callbacks[dict_entry_name](newline, dict_entry_name, _vue)
	return newline


def process_setter_line(_line, _vue=-1):
	code_line = con_line[_line.find("(") + 1:_line.find(")")]

	# multiple SYS dial patch
	if code_line.find('Dial=SYS') > -1 and code_line.find('-') > -1:
		_line_s = code_line.split('-')
		code_line = _line_s[0] + '-SYS' + _line_s[1]

	code_line = preprocess_setter_line(code_line, _vue)

	# multiple SYS dial patch
	if _line.find('/set(Dial=SYS') > -1 and _line.find('-') > -1:
		_line_s = _line.split('-')
		_line = _line_s[0] + ',SYS' + _line_s[1]

	operation_type = "="

	if _line.find('=') == -1:
		if _line.find('+') > -1: # /set(Flag037+1)
			l_op = code_line.split("+")[0].replace("(", "") # Ex: "Flag037"
			r_op = code_line.split("+")[1].replace(")", "") # Ex: "1"
			operation_type = "+"
	else:
		l_op = code_line.split("=")[0].replace("(", "") # Ex: "HideSpr"
		r_op = code_line.split("=")[1].replace(")", "") # Ex: "Poisson"
	if r_op.isdigit():
		r_op = str(int(r_op))
	else:
		if r_op.find(',') > -1:
			r_op_clean = ''
			for r_sub_op_idx, r_sub_op in enumerate(r_op.split(',')):
				if r_sub_op.isdigit():
					r_sub_op = str(int(r_sub_op))
				r_op_clean += r_sub_op
				if r_sub_op_idx < len(r_op.split(',')) - 1:
					r_op_clean += ','
			r_op = r_op_clean

	negative_term = False
	if l_op in athanor_set_dict:
		if "input" in athanor_set_dict[l_op] and athanor_set_dict[l_op]["input"] == "game_object":
			if r_op.startswith("NOT_"):
				negative_term = True
				r_op = r_op.replace("NOT_", "")
			r_op = athanor_var(r_op)
		elif "collect_to_table" in athanor_set_dict[l_op]:
			if "collect_prefix" in athanor_set_dict[l_op]:
				r_op = athanor_set_dict[l_op]["collect_prefix"] + r_op
			if "variable_type" in athanor_set_dict[l_op] and athanor_set_dict[l_op]["variable_type"] == "string":
				r_op = '"' + r_op + '"'
			globals()[athanor_set_dict[l_op]["collect_to_table"]].append(r_op)

		if athanor_set_dict[l_op]["type"] == "function":
			nominal_op_count = 1
			if "nominal_op_count" in athanor_set_dict[l_op]:
				nominal_op_count = athanor_set_dict[l_op]["nominal_op_count"]
			if r_op.count(',') == nominal_op_count: # means more than the nominal amount of parameters
				function_suffix = '_ex'
				_sanitized_r_op = ''
				for _idx, _sub_r_op in enumerate(r_op.split(',')):
					if _sub_r_op.isdigit():
						_sub_r_op = str(int(_sub_r_op))
					_sanitized_r_op += _sub_r_op
					if _idx < len(r_op.split(',')) - 1:
						_sanitized_r_op += ','
				r_op = _sanitized_r_op

			else:
				function_suffix = ''
			if "variable_translator" in athanor_set_dict[l_op]:
				r_op = athanor_set_dict[l_op]["variable_translator"](r_op)
			pre_params = ""
			if "param" in athanor_set_dict[l_op]:
				pre_params = athanor_set_dict[l_op]["param"] + ", "
			code_line = athanor_set_dict[l_op]["name"] + function_suffix + "(" + pre_params + ("", "-")[negative_term] + str(r_op) + ")"
		else:
			code_line = athanor_set_dict[l_op]["name"] + " = " + r_op
	elif l_op in athanor_con_dict:
		if operation_type == '+':
			code_line = code_line.replace('+', ' += ')

	# for script_token in athanor_set_dict:
	# 	code_line = code_line.replace(script_token, athanor_set_dict[script_token]["name"])
	# code_line = code_line.replace("=", "(")
	code_line += ";"
	return code_line

# Condition /CON /SET processing #################
vue_condition_str = ""
vue_condition_h_str = ""

for con_block_description in [["master", "finmaster"], ["timers", "fintimers"]]:

	# # Specific master/timer conditions
	# f_str = ""
	# f_str += "/* Master conditions */\n"
	#
	# f_str += "\n"
	# vue_condition_str += f_str
	block_in = con_block_description[0]
	block_out = con_block_description[1]

	f_str = ""
	f_str += "/* " + block_in.capitalize() + " conditions */\n"

	f_str += "void condition_" + block_in + "(void)\n"
	f_str += "{\n"

	f_str += "\tswitch(world_get_current_index())\n"
	f_str += "\t{\n"

	conditions = {}
	for world_idx, condition_script_native_file in enumerate(condition_script_native_files):
		condition_script_short_filename = condition_script_native_file.split("/")[-1]
		condition_is_open = False
		within_timers_sequence = False
		with open(condition_script_native_file, encoding='cp437') as con_raw:
			con_raw_str = con_raw.readlines()
			for con_line in con_raw_str:
				con_line = con_line.strip()
				if con_line is not None and len(con_line) > 0:
					mode = con_line[0]
					if mode == '/':
						condition_command = con_line[1:]
						if con_line[1:].lower() == block_in: # "timers"
							within_timers_sequence = True
							f_str += "\t\tcase world_" + chapter_dict[world_idx + 1] + ": /* (" + condition_script_short_filename + ", " + chapter_dict[world_idx + 1] + ") */\n"
						elif con_line[1:].lower() == block_out: # "fintimers"
							within_timers_sequence = False
							if condition_is_open:
								f_str += "\t\t\t}\n"
							f_str += "\t\t\tbreak; /* End of " + condition_script_short_filename + " */\n\n"
							# f_str += "\t\treturn;\n"
						# SafarScript condition code
						elif within_timers_sequence and con_line[1:4] == "con":
							if condition_is_open:
								f_str += "\t\t\t\treturn;\n"
								f_str += "\t\t\t}\n"
							code_line = process_condition_line(con_line)

							code_line += "\n" + "\t\t\t{\n"
							f_str += "\t\t\t" + code_line
							condition_is_open = True
						# SafarScript execution code
						elif within_timers_sequence and con_line[1:4] == "set":
							code_line = process_setter_line(con_line)
							f_str += "\t\t\t\t" + code_line + "\n"

	f_str += "\t}\n"
	f_str += "}\n"

	f_str += "\n"
	vue_condition_str += f_str

# Generic "vue conditions"
vue_condition_str += "/* Vue conditions */\n"

within_init_sequence = False
conditions = {}
condition_within_vue = False
condition_is_open = False
for condition_script_native_file in condition_script_native_files:
	condition_script_short_filename = condition_script_native_file.split("/")[-1]
	with open(condition_script_native_file, encoding='cp437') as con_raw:
		con_raw_str = con_raw.readlines()
		for con_line in con_raw_str:
			con_line = con_line.strip()
			if con_line is not None and len(con_line) > 0:
				mode = con_line[0]
				if mode == '/':
					condition_command = con_line[1:]
					# /25, /26, ... conditions for a vue
					if condition_command.isdigit() and int(condition_command) > 0:
						condition_within_vue = True
						vue_index = int(condition_command)
						temp_condition = {"condition_list": []}
						if str(vue_index) in conditions:
							temp_condition = conditions[str(vue_index)]
						f_str = ""
						f_str += "\t/* vue-specific condition (" + condition_script_short_filename + ") */\n"
						f_str += "#ifdef DEBUG_MACROS\n"
						f_str += "\tif(" + athanor_con_dict["ClicZone"]["name"] + " > -1)\n"
						f_str += "\t\tprintf(\"conditions for vue #" + str(vue_index) + ", zone = %d/%s, spr = %d\\n\", " + athanor_con_dict["ClicZone"]["name"] + ", game_object_name[" + athanor_con_dict["ClicZone"]["name"] + "], " + athanor_con_dict["ClicPerso"]["name"] + ");\n"
						f_str += "\telse\n"
						f_str += "\t\tprintf(\"conditions for vue #" + str(vue_index) + ", zone = %d, spr = %d\\n\", " + athanor_con_dict["ClicZone"]["name"] + ", " + athanor_con_dict["ClicPerso"]["name"] + ");\n"
						f_str += "#endif\n"
					# SafarScript init condition
					elif con_line[1:].lower() == "init":
						within_init_sequence = True
						f_str += "\tif(init_vue)\n\t{\n"
					elif con_line[1:].lower() == "fininit":
						within_init_sequence = False
						f_str += "\t\t} /* Last test of the init sequence */\n"
						# f_str += "\t\treturn;\n"
					# SafarScript condition code
					elif condition_within_vue and con_line[1:4] == "con":
						if condition_is_open:
							# if not within_init_sequence:
							f_str += _indent(within_init_sequence) + "\t\treturn TRUE;\n"
							f_str += _indent(within_init_sequence) + "\t}\n"
						code_line = process_condition_line(con_line)

						code_line += "\n" + _indent(within_init_sequence) + "\t{\n"
						f_str += _indent(within_init_sequence) + "\t" + code_line
						condition_is_open = True
					# SafarScript execution code
					elif condition_within_vue and con_line[1:4] == "set":
						code_line = process_setter_line(con_line, vue_index)
						f_str += _indent(within_init_sequence) + "\t\t" + code_line + "\n"
					elif condition_within_vue and condition_command.isdigit() and int(condition_command) == 0:
						f_str += _indent(within_init_sequence) + "\t\treturn TRUE;\n\t}\n"
						temp_condition["condition_list"].append(f_str)
						conditions[str(vue_index)] = temp_condition
						condition_is_open = False
						condition_within_vue = False

# remove duplicates
music_symbols = list(set(music_symbols))
music_symbols = sorted(music_symbols)

_t = []
for sample_vue_key in sorted(sample_vue_table):
	_t.append({"vue": sample_vue_table[sample_vue_key]["vue"], "index":sample_vue_table[sample_vue_key]["index"]})
sample_vue_table = _t

# print(sample_vue_table)

for vue in zones:
	for zone in zones[vue]["zone_list"]:
		if not(str(zone["vue"]) in conditions):
			conditions[str(zone["vue"])] = {"condition_list": ["\t/* stub condition code */\n"]}

for vue_index in conditions:
	vue_condition_h_str += "BOOL conditions_vue_" + vue_index + "(BOOL, short, short);\n"

	f_str = ""
	f_str += "/* vue " + vue_index + " */\n"
	f_str += "BOOL conditions_vue_" + vue_index + "(BOOL init_vue, short " + athanor_con_dict["ClicZone"]["name"] + ", short " + athanor_con_dict["ClicPerso"]["name"] + ")\n"
	f_str += "{\n"

	for condition in conditions[vue_index]["condition_list"]:
		f_str += condition

	f_str += "\treturn FALSE;\n}\n\n"
	vue_condition_str += f_str

# Objects combined
# game_object_combined
object_combine_pairs = []
for combined_list in game_object_combined:
	with open(combined_list, encoding='cp437') as combined_raw:
		combined_raw_str = combined_raw.readlines()
		for object_pair in combined_raw_str:
			object_pair = object_pair.strip().split(' ')
			object_combine_pairs.append({"in": [object_pair[0], object_pair[1]], "out": object_pair[2]})

print(object_combine_pairs)

##################################
#            ROOMS AND
#         WORLD STRUCTURE
##################################

# codage des vues :
# (1=N; 2=S; 8=E; 4=O)
#
# 25,cnos025,09290026
# 26,cnos026,1200002725
#
# Ou par exemple pour la vue25 codage :
# 09 = Deux issues ouvertes (N et E)
# 29 = Issue vers le N
# 00 = Issue vers le S
# 26 = Issue vers l ' E
#
# Ou par exemple pour la vue26 codage :
# 12 = Deux issues ouvertes (E et O)
# 00 = Issue vers le N
# 00 = Issue vers le S
# 27 = Issue vers l ' E
# 25 = Issue vers l ' O
#
# Codage des SPR :
#
# SPR.id$       = Id (ex = LILLA) (C'est cet ID qui est repris dans les scriptes de scénar) (char)
# SPR.vue&   = Spr visible dans cette vue (word)
# SPR.num|  = Numéro dans la bank (byte)
# SPR.anim| = N° de l'anim (moteur v2.0) (byte)
# SPR.suj$    = Sujets de dial. ouverts (moteur v2.0) (char)
# SPR.onoff! = Visible or Not (bool)


# create an intermediary World Json file based on the GFA basic source
if False:
	parsed_world_dict = parse_vues_links(gfa_listing_file)
	out = open(world_file, "w")
	out.write(json.dumps(parsed_world_dict, sort_keys=False, indent=4))
	out.close()

# parse the json world file
if os.path.exists(world_file):
	with open(world_file) as json_data:
		out = open(out_c_file, "w")
		out_h = open(out_h_file, "w")

		world = json.loads(json_data.read(), encoding="UTF8")

		# enumerate rooms, object & verbs listed in a sorted way
		for room_name in sorted(world["rooms"].keys()):
			room_names.append(room_name)

		# sort by alphanum
		room_names.sort()
		# object_names.sort()

		# C initialization
		out.write('/* Athanor2 (C) Eric "Atlantis" Safar / Safargames 2021 */\n')
		out.write('/* THIS CODE WAS GENERATED BY R-PAGE SCENARIO TOOLCHAIN */\n')
		out.write('/*                   DON\'T MODIFY IT !                  */\n\n')
		out.write('#include <stdarg.h>\n')
		out.write('#include <stdio.h>\n')
		out.write('#include <string.h>\n')
		out.write('#include <stdlib.h>\n')
		out.write('#include "rpage/frwk.h"\n')
		out.write('#include "game/text.h"\n')
		out.write('#include "game/world.h"\n')
		out.write('#include "game/special.h"\n')
		# out.write('#include "game/world_const.h"\n\n')

		# Game initial state (where the player is spawned...)
		out.write('\n')

		if 'room' in world['init']:
			out.write('short current_room = ' + str(room_names.index(world['init']['room'])) + ';\n')
		else:
			raise ValueError('No current_room defined.')

		out.write('short previous_vue = ' + str(-1) + ';\n')
		out.write('short current_world = world_' + world_dict[1] + ';\n')

		# World data
		# Collect all variables
		variables = []
		for zone in zones:
			for j in range(len(zones[zone]["zone_list"])):
				variables.append(zones[zone]["zone_list"][j]["variable"])

		variables = list(set(variables))

		# merge sprite names to variables
		for i in range(len(sprites)):
			if not(sprites[i]['sprite_name'] in variables):
				variables.append(sprites[i]['sprite_name'])

		# merge inventory objects to variables
		for i in range(len(inventory_objects)):
			if not(inventory_objects[i]['object_name'] in variables):
				variables.append(inventory_objects[i]['object_name'])

		# merge combined objects to variables
		for combined_object in object_combine_pairs:
			if not(combined_object["out"] in variables):
				variables.append(combined_object["out"])

		for _var in athanor_missing_vars:
			variables.insert(0, _var)
		if "VIDE" in variables:
			variables.remove("VIDE")
		variables.sort()
		variables.insert(0, "VIDE")

		# # Writes all sprites
		# out.write('\n')
		# out.write('/***********/\n')
		# out.write('/* Sprites */\n')
		# out.write('/***********/\n')
		#
		# if len(sprites) > 0:
		# 	# enum
		# 	out.write('\n')
		# 	out.write('enum game_sprite {')
		# 	for i in range(len(sprites)):
		# 		out.write("spr_" + sprites[i]['sprite_name'])
		# 		if i < len(sprites) - 1:
		# 			out.write(', ')
		# 			if (i + 2)%4 == 0:
		# 				out.write('\n\t')
		# 	out.write('};\n')

		# World constants
		out_h.write('/* Athanor2 (C) Eric "Atlantis" Safar / Safargames 2021 */\n')
		out_h.write('/* THIS CODE WAS GENERATED BY R-PAGE SCENARIO TOOLCHAIN */\n')
		out_h.write('/*                   DON\'T MODIFY IT !                  */\n\n')
		out_h.write('#ifndef _WORLD_CONSTANTS_\n')
		out_h.write('#define _WORLD_CONSTANTS_\n')
		out_h.write('\n')
		out_h.write('#define ATARI_DATASET_VERSION "' + atari_version + '"\n')
		out_h.write('\n')
		out_h.write('/* World constants */\n')
		out_h.write('#define MAX_ROOM ' + str(len(world["rooms"])) + '\n')
		out_h.write('#define MAX_ZONE ' + str(zone_max) + '\n')
		out_h.write('#define MAX_WORLD ' + str(len(world_dict)) + '\n')
		out_h.write('#define MAX_SPR_PORTRAITS ' + str(len(portraits["portraits"])) + '\n')
		out_h.write('#define MAX_GAME_OBJECTS ' + str(len(variables)) + '\n')
		out_h.write('#define MAX_COMBINED_OBJECTS ' + str(len(object_combine_pairs)) + '\n')
		out_h.write('\n')

		# # sample per vue
		# if len(sample_vue_table) > 0:
		# 	out_h.write('/* sample for each vue */\n')
		# 	for sample_vue in sample_vue_table:
		# 		{ }

		# music names
		if len(music_symbols) > 0:
			out_h.write('/* musics enum */\n')
			if enum_instead_of_defines:
				_enum_str = ""
				for music_str in music_symbols:
					_enum_str += music_str + ', '
				_enum_str = _enum_str[:-2]
				out_h.write('enum music {' + _enum_str + '};\n\n')
			else:
				for _idx, music_str in enumerate(music_symbols):
					out_h.write('#define ' + music_str + ' ' + str(_idx) + '\n')
				out_h.write('\n')

		out_h.write('/* world enum */\n')
		if enum_instead_of_defines:
			_enum_str = ""
			for world_str in world_dict:
				_enum_str += 'world_' + world_str + ', '
			_enum_str = _enum_str[:-2]

			out_h.write('enum world {' + _enum_str + '};\n')
		else:
			for _idx, world_str in enumerate(world_dict):
				out_h.write('#define world_' + world_str + ' ' + str(_idx) + '\n')
			out_h.write('\n')

		# game object names
		out_h.write('#ifdef DEBUG_MACROS\n')
		out_h.write('extern const char *game_object_name[MAX_GAME_OBJECTS];\n')
		out_h.write('#endif\n')

		out_h.write('\n')
		out_h.write('extern const unsigned short game_object_crc16[MAX_GAME_OBJECTS];\n')

		# portraits sprites
		out_h.write('\n')
		out_h.write("extern const char *portrait_sprites[MAX_SPR_PORTRAITS];\n")

		# # objects combined
		# out_h.write("extern combined_game_object combined_go[MAX_COMBINED_OBJECTS];\n")

		# Write all conditions headers
		out_h.write('\n')
		out_h.write(vue_condition_h_str)
		out_h.write('\n')
		out_h.write('void worldClearScenarioFlags(void);\n')
		out_h.write('void worldDebugScenarioFlags(char *_str);\n')
		out_h.write('\n')

		# Writes all variables
		out_h.write('/**********************/\n')
		out_h.write('/* Game Objects (go_) */\n')
		out_h.write('/**********************/\n')

		# dumping the so-called "game objects"
		if enum_instead_of_defines:
			if zone_max > 0 or len(variables) > 0:
				# enum
				out_h.write('\n')
				out_h.write('enum game_object {')
				for i in range(len(variables)):
					out_h.write(athanor_var(variables[i]))
					if i < len(variables) - 1:
						out_h.write(', ')
						if (i + 2)%4 == 0:
							out_h.write('\n\t')
				out_h.write('};\n')
		else:
			if zone_max > 0 or len(variables) > 0:
				for i in range(len(variables)):
					out_h.write("#define " + athanor_var(variables[i]) + " " + str(i) + "\n")

		if zone_max > 0:
			# enum
			out.write('\n')
			out.write('#ifdef DEBUG_MACROS\n')
			out.write('const char *game_object_name[MAX_GAME_OBJECTS] = {')
			for i in range(len(variables)):
				out.write('"' + variables[i] + '"')
				if i < len(variables) - 1:
					out.write(', ')
					if (i + 2)%4 == 0:
						out.write('\n\t')
			out.write('};\n')
			out.write('#endif\n')
			out.write('\n')

			out.write('const unsigned short game_object_crc16[MAX_GAME_OBJECTS] = {\n\t')
			for i in range(len(variables)):
				# shortname = ''.join([l for l in variables[i] if l not in ('A', 'E', 'I', 'O', 'U')])
				# hash = crc8.crc8()
				# hash.update(variables[i].encode('ascii'))
				# shortname = hash.hexdigest()
				shortname = hex_crc16(variables[i]) # hex(crc16.crc16xmodem(str_in.encode('ascii')))
				# shortname = shortname.replace('0x', '')
				# shortname = ('0' * (4 - len(shortname))) + shortname
				out.write('' + shortname + '')
				if i < len(variables) - 1:
					out.write(', ')
					if (i + 2)%4 == 0:
						out.write('\n\t')
			out.write('};\n')

		out.write('\n')

		# Combined game objects
		out.write('/* Combined objects */\n')
		out.write('combined_game_object combined_go[MAX_COMBINED_OBJECTS] = {\n')
		out.write('/* \t{ { go_A, go_B }, go_C },\n')
		out.write('\t      A  +  B  -->  C\n*/\n')
		out_str = ''
		for combined_object in object_combine_pairs:
			in_a = combined_object["in"][0]
			in_b = combined_object["in"][1]
			if in_b < in_a:
				in_a, in_b = in_b, in_a
			out_str += '\t{ {' + athanor_var(in_a) + ', ' + athanor_var(in_b) + '}, ' + athanor_var(combined_object["out"]) + ' },\n'
		out_str = out_str[:-2]
		out_str += '\n};\n'

		out.write(out_str)

		# Sprites filenames by world
		out.write('\n')
		out.write('/****************************/\n')
		out.write('/* Sprites sheets per world */\n')
		out.write('/****************************/\n')

		_spr_str = ""
		for world_str in world_dict:
			_spr_str += '"spr_' + world_str + '", '

		_spr_str = _spr_str[:-2]

		out.write('const char *world_sprites_name[' + str(len(world_dict)) + '] =\n{\n')
		out.write('\t' + _spr_str + '\n')
		out.write('};\n')
		out.write('\n')

		# read Json inventory sprites
		max_inventory_sprite = 0
		game_object_to_inventory_sprite = [-1] * len(variables)

		if os.path.exists(sprites_inventory_file):
			with open(sprites_inventory_file) as json_data:
				sprites_inventory_dict = json.loads(json_data.read(), encoding="UTF8")

		for sprite_index, inventory_spr in enumerate(sprites_inventory_dict["sprites"]):
			game_object_to_inventory_sprite_idx = -1
			for i in range(len(variables)):
				if inventory_spr == athanor_var(variables[i]):
					game_object_to_inventory_sprite[i] = sprite_index
					break

		# read Json sprite sheets
		max_sheet_sprite = 0
		game_object_to_sheet_sprite = [-1] * len(variables)

		for sprite_coord in sprites_coord_files:
			if os.path.exists(sprite_coord):
				with open(sprite_coord) as json_data:
					sprites_coords_dict = json.loads(json_data.read(), encoding="UTF8")
					# print(sprites_coords_dict)
					max_sheet_sprite += len(sprites_coords_dict["sprites"])

		sprite_sheet_str = "sheet_sprite world_sprites[" + str(max_sheet_sprite) + "] = \n"
		sprite_sheet_str += "{\n"
		sprite_index = 0

		for sprite_coord in sprites_coord_files:
			if os.path.exists(sprite_coord):
				with open(sprite_coord) as json_data:
					sprites_coords_dict = json.loads(json_data.read(), encoding="UTF8")
					sprite_sheet_str += "\t/* Sprite sheet for '" + sprites_coords_dict["world"] + "' */\n"
					# print(sprites_coords_dict)
					for _idx, _val in enumerate(sprites_coords_dict["sprites"]):
						sprite_vue = -1
						sprite_enabled = "FALSE"
						for _sprite in sprites:
							if athanor_var(_sprite["sprite_name"]) == _val["name"]:
								sprite_vue = _sprite["data"]["vue"]
								sprite_enabled = _sprite["data"]["enabled"]
								if sprite_enabled:
									sprite_enabled = "TRUE"
								else:
									sprite_enabled = "FALSE"

						sprite_state_save_load.append({"name":_val["name"], "enabled": sprite_enabled}) # save this for later (see save/load routine generation)

						sprite_sheet_str += "\t{\n"
						sprite_sheet_str += "\t\t/* " + _val["name"] + "*/\n"
						sprite_sheet_str += "\t\t{" + str(_val["source"][0]) + ", " + str(_val["source"][1]) + ", " + str(_val["source"][2]) + ", " + str(_val["source"][3]) + "},\n"
						sprite_sheet_str += "\t\t{" + str(_val["destination"][0]) + ", " + str(_val["destination"][1]) + "},\n"
						sprite_sheet_str += "\t\t" + str(sprite_vue) + ",\n"
						sprite_sheet_str += "\t\t" + str(sprite_enabled) + ",\n"
						sprite_sheet_str += "\t\t" + str(sprite_enabled) + ", /* default value */\n"
						if _val["name"].startswith("go"):
							sprite_sheet_str += "\t\t" + _val["name"] + "\n"
						else:
							sprite_sheet_str += "\t\t" + str(-1) + "\n"
						sprite_sheet_str += "\t},\n"

						game_object_to_sheet_sprite_idx = -1
						for i in range(len(variables)):
							if _val["name"] == athanor_var(variables[i]):
								game_object_to_sheet_sprite[i] = sprite_index
								break

						sprite_index += 1

		sprite_sheet_str += "};\n"
		sprite_sheet_str += "\n"
		out.write(sprite_sheet_str)

		out.write('\n')
		out.write('/*************************************/\n')
		out.write('/* Game Object to Sheet Sprite table */\n')
		out.write('/*************************************/\n')

		out.write("const short go_to_sprite[" + str(len(game_object_to_sheet_sprite)) + "] = \n")
		out.write("{\n")
		for _idx, _val in enumerate(game_object_to_sheet_sprite):
			if _val > -1:
				out.write('\t' + str(_val) + ', /* ' + athanor_var(variables[_idx]) + ' */\n')
			else:
				out.write('\t' + str(_val) + ',\n')
		out.write("};\n")

		out.write('\n')
		out.write('/****************************************/\n')
		out.write('/* Game Object to Inventory Sprite Index*/\n')
		out.write('/****************************************/\n')
		out.write("const short go_to_inventory_sprite[" + str(len(game_object_to_inventory_sprite)) + "] = \n")
		out.write("{\n")
		for _idx, _val in enumerate(game_object_to_inventory_sprite):
			if _val > -1:
				out.write('\t ' + str(_val) + ', /* ' + athanor_var(variables[_idx]) + ' */\n')
			else:
				out.write('\t' + str(_val) + ',\n')
		out.write("};\n")

		##################################
		#    INVENTORY OBJECTS TOOLTIP
		##################################

		out.write('/****************************************/\n')
		out.write('/* Game Object to Inventory Tooltip Index*/\n')
		out.write('/****************************************/\n')
		out.write("const short go_to_inventory_tooltip[MAX_GAME_OBJECTS] = \n")
		out.write("{\n")

		# _idx = 0
		go_key_list = []
		for go_key in sprites_inventory_dict["sprites"]:
			if go_key.lower().startswith("go_"):
				go_key_list.append(go_key)

		for i in range(len(variables)):
			go_tooltip_index = -1
			for tooltip_index, go_key in enumerate(go_key_list):
				if go_key == athanor_var(variables[i]):
					go_tooltip_index = tooltip_index

			# out.write('\t' + athanor_var(variables[_idx]) + ', /* ' + str(go_tooltip_index) + ' */\n')
			if go_tooltip_index > -1:
				out.write('\t ' + str(go_tooltip_index) + ', /* ' + go_key_list[go_tooltip_index] + ' */\n')
			else:
				out.write('\t' + str(go_tooltip_index) + ', \n')

		out.write("};\n")

		##################################
		# 		SAMPLES
		##################################

		# sample_name_table = []
		# sample_name_idx = -1

		# if len(samples) > 0:
		# 	out.write('\n')
		# 	out.write('/***********/\n')
		# 	out.write('/* Samples */\n')
		# 	out.write('/***********/\n')
		# 	out.write('vue_sample vue_samples[SAMPLE_VUE_LEN] =\n{\n')
		# 	for _idx, sample in enumerate(samples):
		# 		# if not (samples[_idx]['name'] in sample_name_table):
		# 		# 	sample_name_table.append(samples[_idx]['name'])
		# 		#
		# 		# sample_name_idx = sample_name_table.index(samples[_idx]['name'])
		#
		# 		out.write('\t{ ' + str(samples[_idx]['vue']) + ', ' + str(sample_name_idx) + ' }, /* ' + samples[_idx]['name'] + ' */\n')
		# 	out.write('};\n')
		# 	out.write('\n')

		if len(samples) > 0:
			out.write('\n')
			out.write('/***********/\n')
			out.write('/* Samples */\n')
			out.write('/***********/\n')

			out.write('sample_per_world world_samples[SAMPLE_WORLD_LEN] =\n{\n')

			for world_sample in samples:
				for sample in samples[world_sample]:
					# print(sample)
					sample_filename_idx = sample_name_table.index(sample['name'])
					out.write('\t{ world_' + str(world_sample) + ', ' + str(sample['index']) + ', ' + str(sample_filename_idx) + ' }, /* ' + str(sample['name']) + ' */\n')

			out.write('};\n')
			out.write('\n')

		if len(sample_vue_table) > 0:
			out.write('\n')
			out.write('/************************/\n')
			out.write('/* Vue/Samples relation */\n')
			out.write('/************************/\n')

			out.write('sample_per_vue vue_samples[SAMPLE_VUES_LEN] =\n{\n')

			for sample in sample_vue_table:
				_i = sample["vue"]
				_s = sample["index"]
				out.write('\t{ ' + str(_i) + ', ' + str(_s) + '}, /* vue ' + str(_i) + ', sample #' + str(_s) + ' */ \n')

			out.write('};\n')
			out.write('\n')

		if len(samples) > 0:
			out.write('/*********************/\n')
			out.write('/* Samples filenames */\n')
			out.write('/*********************/\n')
			out.write('char *sample_names[SAMPLE_NAMES_LEN] =\n{\n')
			for _idx, sample in enumerate(sample_name_table):
				out.write('\t"sfx_' + sample + '",\n')
			out.write('};\n')

			out_h.write('\n')
			out_h.write('/* Samples */\n')
			sample_world_len = 0
			for world_sample in samples:
				for sample in samples[world_sample]:
					sample_world_len += len(sample)
			out_h.write('#define SAMPLE_WORLD_LEN ' + str(sample_world_len) + '\n')
			out_h.write('#define SAMPLE_NAMES_LEN ' + str(len(sample_name_table)) + '\n')
			out_h.write('#define SAMPLE_VUES_LEN ' + str(len(sample_vue_table)) + '\n')

		##################################
		#       SPRITES PORTRAITS
		##################################

		if portraits is not None:
			out.write('\n')
			out.write('/********************************/\n')
			out.write('/* Portraits sprites definition */\n')
			out.write('/********************************/\n')
			out.write("const char *portrait_sprites[MAX_SPR_PORTRAITS] = \n")
			out.write('{\n')
			for portrait in portraits["portraits"]:
				out.write('\t"npc_{:s}_{:02d}",\n'.format(portrait["name"], portrait["vue"]))
			out.write('};\n')

		out.write('\n')
		out.write('/******************/\n')
		out.write('/* Scenario flags */\n')
		out.write('/******************/\n')

		flag_amount = sum(("explicit_declaration" in athanor_con_dict[con_var] and athanor_con_dict[con_var]["explicit_declaration"]) for con_var in athanor_con_dict)
		flag_list = []
		out.write('\n')
		out.write('short ')
		for con_var in athanor_con_dict:
			if "explicit_declaration" in athanor_con_dict[con_var] and athanor_con_dict[con_var]["explicit_declaration"]:
				out.write(con_var)
				flag_list.append(con_var)
				flag_amount -= 1
				if flag_amount > 0:
					out.write(', ')
		out.write(';\n')

		# Write all conditions
		out.write('\n')
		out.write('/**************************/\n')
		out.write('/* SafarScript Conditions */\n')
		out.write('/**************************/\n')
		out.write('\n')

		out.write(vue_condition_str)

		# Writes all zones
		out.write('\n')
		out.write('/***********/\n')
		out.write('/*  Zones  */\n')
		out.write('/***********/\n')


		def rect_to_poly(rect):
			sx = rect[0]
			sy = rect[1]
			ex = rect[2]
			ey = rect[3]
			return [sx, sy, ex, sy, ex, ey, sx, ey]

		if zone_max > 0:
			out.write('\n')
			out.write('zone zones[MAX_ZONE] = {\n')
			out.write('\t/* { zone rect, object enum index, object variable pointer, condition code functer }*/\n')
			for i in range(zone_max):
				for zone in zones:
					for j in range(len(zones[zone]["zone_list"])):
						if zones[zone]["zone_list"][j]["index"] == i:
							z = zones[zone]["zone_list"][j]
							zr = [str(x) for x in z["rect"]] # rect of the zone
							zp = rect_to_poly(zr) # we turn the rect into a polygon
							# zone_str = '{' + zr[0] + ', ' + zr[1] + ', ' + zr[2] + ', ' + zr[3] + '}, '
							zone_str = '{'
							zone_str += '{' + zp[0] + ', ' + zp[1] + '}, '
							zone_str += '{' + zp[2] + ', ' + zp[3] + '}, '
							zone_str += '{' + zp[4] + ', ' + zp[5] + '}, '
							zone_str += '{' + zp[6] + ', ' + zp[7] + '}'
							zone_str += '}, '
							out.write('\t{' + zone_str + athanor_var(z["variable"]) + ', ' + "&conditions_vue_" + str(z["vue"]))
							out.write('}, \t\t/* zone #' + str(i) + ' */\n')

		out.write('};\n\n')

		# collect all rooms
		print("found " + str(len(room_names)) + " rooms!")
		out.write('/***********/\n')
		out.write('/*  Rooms  */\n')
		out.write('/***********/\n')

		out.write('\n')

		# save_links_table = []

		i = 0
		diagnostic_MAX_ZONE_PER_ROOM = 0
		out.write('room rooms[MAX_ROOM] = {\n')
		for room_name in room_names:
			# print(room_name)
			vue_index = -1
			out.write('\t{ /*    ' + str(room_name).upper() + '    */\n')
			if "vue" in world["rooms"][room_name]:
				vue_index = world["rooms"][room_name]["vue"]
				out.write('\t\t' + str(vue_index) + ',\n')

			# # room name
			# if "short" in world["rooms"][room_name]:
			# 	out.write('\t\t"' + world["rooms"][room_name]["short"] + '",\n')

			# bitmap
			bitmap_filename = ''
			if "bitmap" in world["rooms"][room_name]:
				bitmap_filename = world["rooms"][room_name]["bitmap"][:MAX_FILENAME_CHAR_LEN]
				if FILENAME_OBFUSCATION_ENABLED:
					bitmap_filename = obfuscate_filename(FILENAME_SALT, bitmap_filename)
				bitmap_filename = bitmap_filename.split('.')[0]
			out.write('\t\t"' + bitmap_filename + '",\n')

			# links to the other rooms
			links = [None] * len(enum_link_direction)
			if "links" in world["rooms"][room_name]:
				j = 0
				for link_dir in enum_link_direction:
					if link_dir in world["rooms"][room_name]["links"]:
						links[j] = world["rooms"][room_name]["links"][link_dir]["room"]
						# link_index = room_names.index(link_to_room_name)
					j += 1

			out.write('\t\t{')
			str_comment = '/* '
			for j in range(len(enum_link_direction)):
				link_index = -1
				if links[j] is not None:
					link_index = room_names.index(links[j])
					str_comment += enum_link_direction[j] + ':' + links[j].upper() + ','
				out.write(str(link_index))
				if j < len(enum_link_direction) - 1:
					out.write(',')

			str_comment += ' */'
			out.write('},')
			out.write('\t' + str_comment)
			out.write('\n')

			# save_links_table.append({"vue_index": vue_index, "links": links})

			# click zones
			room_zones = [-1] * MAX_ZONE_PER_ROOM
			room_zones_variables = [""] * MAX_ZONE_PER_ROOM # for C comment only
			if vue_index >= 0 and str(vue_index) in zones:
				i = 0
				for i, zone in enumerate(zones[str(vue_index)]["zone_list"]):
					room_zones[i] = zone["index"]
					room_zones_variables[i] = zone["variable"]

			out.write('\t\t{')
			str_comment = '/* '
			local_MAX_ZONE_PER_ROOM = 0
			for j in range(len(room_zones)):
				if room_zones[j] >= 0:
					str_comment += str(room_zones_variables[j])
					local_MAX_ZONE_PER_ROOM += 1
				str_comment += ','
				out.write(str(room_zones[j]))
				if j < len(room_zones) - 1:
					out.write(',')

			diagnostic_MAX_ZONE_PER_ROOM = max(diagnostic_MAX_ZONE_PER_ROOM, local_MAX_ZONE_PER_ROOM)

			str_comment += ' */'

			out.write('},')

			out.write('\t' + str_comment + '\n')

			# condition code
			if "vue" in world["rooms"][room_name]:
				vue_index = world["rooms"][room_name]["vue"]
				if str(vue_index) in conditions:
					out.write('\t\t&conditions_vue_' + str(vue_index) + '\n')
				else:
					out.write('\t\tNULL' + '\n')

			# out.write('\n')

			out.write('\t},\n')
			out.write('\n')
			i += 1
		out.write('\t/* End of rooms list */\n')
		out.write('};\n')
		out.write('\n')

		out.write('\n')

		print("diagnostic_MAX_ZONE_PER_ROOM = " + str(diagnostic_MAX_ZONE_PER_ROOM))

		# functions
		out.write('/* Various accessors */\n')

		out.write('short worldGetMaxRoom(void) { return MAX_ROOM; }\n')
		out.write('short worldGetCurrentRoom(void) { return current_room; }\n')
		out.write('void worldSetCurrentRoom(short room_index) { current_room = room_index; }\n')
		out.write('void worldSetCurrentRoomByVue(short vue_index) { \n\
\tshort i;\n\
\tfor(i = 0; i < MAX_ROOM; i++)\n\
\t{\n\
\t\tif (rooms[i].vue_index == vue_index)\n\
\t\t{\n\
\t\t\tcurrent_room = i;\n\
\t\t\tbreak;\n\
\t\t}\n\
\t}\n\
}\n\n')

		out.write('void worldSetCurrentRoomByVue_ex(short vue_index, short dialog_index) {\n')
		out.write('\tgame_set_next_vue_dialog(dialog_index);\n')
		out.write('\tworldSetCurrentRoomByVue(vue_index);\n')
		out.write('}\n\n')

		out.write('short vue_get_room_index(short vue_index) { \n\
\tshort i;\n\
\tfor(i = 0; i < MAX_ROOM; i++)\n\
\t{\n\
\t\tif (rooms[i].vue_index == vue_index)\n\
\t\t\treturn i;\n\
\t}\n\
}\n\n')

		out.write('short roomGetVueIndex(short room_index) { return rooms[room_index].vue_index; }\n')
# 		out.write('short roomGetIndexByName(char *room_name) { /* returns the index of a room if found, -1 otherwise */\n\
# \tshort i;\n\
# \tfor(i = 0; i < MAX_ROOM; i++)\n\
# \t\tif(strncmp(room_name, rooms[i].name, MAX_NAME_CHAR_LEN) == 0) return(i);\n\
# \treturn(-1);\n\
# }\n\n')

		out.write('short roomGetLinkDirection(short room_index, short dir) { return rooms[room_index].links[dir]; }\n')
		out.write('\n')

		out.write('void worldSetCurrentChapter(short chapter_index) { /* stub function */ }\n')
		out.write('void world_set_current_index(short world_index) { current_world = world_index; }\n')
		out.write('short world_get_current_index(void) { return current_world; }\n')
		out.write('#ifdef DEBUG_MACROS\n')
		out.write('char *world_get_object_name(short object_index) { return game_object_name[object_index]; }\n')
		out.write('#endif\n')
		out.write('\n')

		out.write('short getPreviousVue(void)\n')
		out.write('{\n')
		out.write('	return previous_vue;\n')
		out.write('}\n')
		out.write('\n');

		out.write('void setPreviousVue(short v)\n')
		out.write('{\n')
		out.write('	previous_vue = v;\n')
		out.write('}\n')
		out.write('\n')

		out.write('short worldGetMaxSheetSprites(void) { return ' + str(max_sheet_sprite) + ';}\n')
		out.write('\n')

		for _dir in game_cardinal_order:
			out.write('void worldOpenExit' + _dir.title() + '(short from_index, short to_index)\n')
			out.write('{\n')
			out.write('\trooms[vue_get_room_index(from_index)].links[dir_' + str(_dir) + '] = vue_get_room_index(to_index);\n')
			out.write('\tgame_vue_make_dirty();\n')
			out.write('}\n')
			out.write('\n')

		for _dir in game_cardinal_order:
			out.write('void worldCloseExit' + _dir.title() + '(short vue_index)\n')
			out.write('{\n')
			out.write('\trooms[vue_get_room_index(vue_index)].links[dir_' + str(_dir) + '] = -1;\n')
			out.write('\tgame_vue_make_dirty();\n')
			out.write('}\n')
			out.write('\n')

		out.write('void worldClearScenarioFlags(void)\n')
		out.write('{\n')
		for con_var in flag_list:
			default_value = 0
			if "default_value" in athanor_con_dict[con_var]:
				default_value = athanor_con_dict[con_var]["default_value"]
			out.write('\t' + con_var + " = " + str(default_value) + ";\n")
		out.write('}\n')

		out.write('\n')

		out.write('void worldDebugScenarioFlags(char *_str)\n')
		out.write('{\n')
		dbg_str = ''
		var_str = ''
		var_count = 0
		for con_var in flag_list:
			dbg_str += "Flag" + con_var[-2:] + "=%d "
			var_str += con_var + ", "
			var_count += 1
			if var_count >= 4:
				var_count = 0
				dbg_str = dbg_str[:-1] + "\\n"
		var_str = var_str[:-2]
		_str_len = len(dbg_str) * 2
		_str_len = ((_str_len//16) + 1) * 16
		# out.write('\tchar _str[' + str(_str_len) + '];\n')
		out.write('\tsprintf(_str, "' + dbg_str + '", ' + var_str + ');\n')
		out.write('}\n')

		out_h.write('\n')
		out_h.write('#define WRLD_DBG_FLAG_STRSIZE ' + str(_str_len) + '\n')

		out.write('\n')

		out.write('void gameObjectGetSpriteSheetCoords(short go_idx, rect *source, vec2 *dest)\n{\n\tmemcpy(source, &(world_sprites[go_to_sprite[go_idx]].source), sizeof(rect));\n\tmemcpy(dest, &(world_sprites[go_to_sprite[go_idx]].dest), sizeof(vec2));\n}\n')
		out.write('\n')
		out.write('short gameObjectGetSpriteInventoryIndex(short go_idx)\n{\n\treturn go_to_inventory_sprite[go_idx];\n}\n')
		out.write('\n')
		out.write('short getSpriteSheetVueIndex(short idx)\n{\n\treturn world_sprites[idx].vue;\n}\n')
		out.write('\n')
		out.write('BOOL getSpriteSheetEnabled(short idx)\n{\n\treturn world_sprites[idx].enabled;\n}\n')
		out.write('\n')
		out.write('void setSpriteSheetEnabled(short idx, BOOL state)\n{\n\tworld_sprites[idx].enabled = state;\n}\n')
		out.write('\n')
		out.write('short getSpriteSheetGameObjectIndex(short idx)\n{\n\treturn world_sprites[idx].go_index;\n}\n')

		out.write('short worldGetGameObjectFromCRC16(unsigned short value)\n')
		out.write('{\n')
		out.write('    short i;\n')
		out.write('    for(i = 0; i < MAX_GAME_OBJECTS; i++)\n')
		out.write('        if (game_object_crc16[i] == value)\n')
		out.write('            return i;\n')
		out.write('\n')
		out.write('    return OBJECT_NONE;\n')
		out.write('}\n')

		# load / save game
		out.write('\n')
		out.write('void worldSaveScenarioFlags(rpage_file file)\n')
		out.write('{\n')
		out.write('\tUSHORT flag_idx;\n')
		out.write('\tchar *file_header = SAVE_HEADER_FLAGS;\n\n')
		out.write("\trpage_file_write(file, file_header, 4);\n")
		# out.write('/*\n')
		for var in athanor_con_dict:
			if "type" in athanor_con_dict[var] and athanor_con_dict[var]["type"] == "variable":
				if "save" in athanor_con_dict[var] and athanor_con_dict[var]["save"]:
					var_name = athanor_con_dict[var]["name"]
					# out.write("\tsave variable : " + var_name + ";\n")
					var_number = ''.join(filter(str.isdigit, var_name))
					out.write("\tflag_idx = " + str(int(var_number)) + ";\n")
					out.write("\trpage_file_write(file, &flag_idx, 2);\n")
					out.write("\trpage_file_write(file, &" + var_name + ", 2);\n")
		# out.write('*/\n')
		out.write('}\n')

		out.write('\n')

		out.write('void worldResetSpritesState(void)\n')
		out.write('{\n')
		out.write('\tshort i;\n')
		out.write('\tfor(i = 0; i < ' + str(len(sprite_state_save_load)) + '; i++) {\n')
		out.write('\t\tworld_sprites[i].enabled = world_sprites[i].enabled_default;\n')
		out.write('\t}\n')
		out.write('}\n')

		out.write('\n')
		out.write('void worldSaveSpritesState(rpage_file file)\n')
		out.write('{\n')
		out.write('\tunsigned short us;\n')
		out.write('\tshort len;\n')
		out.write('\tUSHORT b;\n\n')
		out.write('\tchar *file_header = SAVE_HEADER_SPRITES;\n')
		out.write("\trpage_file_write(file, file_header, 4);\n")
		out.write("\tlen = " + str(len(sprite_state_save_load)) + ";\n")
		out.write("\trpage_file_write(file, &len, 2);\n")
		# out.write('\tfor(i = 0; i < ' + str(len(sprite_state_save_load)) + '; i++)\n')
		for _idx, _spr in enumerate(sprite_state_save_load):
			_go_hex = hex_crc16(sprite_state_save_load[_idx]['name'].replace('go_', ''))
			out.write('\n')
			out.write('\tus = ' + _go_hex + ';\n')
			out.write('\trpage_file_write(file, &us, 2);\n')
			out.write('\tb = (USHORT)world_sprites[' + str(_idx) + '].enabled;\t/* ' + sprite_state_save_load[_idx]['name'] + ' */\n')
			# out.write('\tprintf("b = %x, ", b);\n')
			out.write('\trpage_file_write(file, &b, 2);\n')
		out.write('}\n')

		out.write('\n')

		out.write('\n')
		out.write('void gameSaveSlot(short slot, BOOL is_on_hdd)\n')
		out.write('{\n')
		# out.write('/*\n')
		out.write('\trpage_file file;\n')
		out.write('\tchar *file_header = SAVE_HEADER;\n')
		out.write('\tchar filename[16];\n')
		out.write('\tif (is_on_hdd)\n')
		out.write('\t\tsprintf(filename, "save.%03d", slot);\n')
		out.write('\telse\n')
		out.write('\t\tsprintf(filename, "%ssave.%03d", SAVE_DISK_NAME, slot);\n')
		out.write('\t// Does the file exists ?\n')
		out.write('\tfile = rpage_file_open(filename, MODE_OPEN_FILE);\n')
		out.write('\tif (!file)\n')
		out.write('\t\tfile = rpage_file_open(filename, MODE_CREATE_FILE);\n')
		out.write("\trpage_file_write(file, file_header, 4);\n")
		out.write("\tgame_save_time(file);\n")
		out.write("\tgameSaveWorldState(file);\n")
		out.write("\tgameSaveInventoryState(file);\n")
		out.write("\tworldSaveScenarioFlags(file);\n")
		out.write("\tworldSaveSpritesState(file);\n")
		out.write("\tgameSaveRoomLinks(file);\n")
		out.write("\tgameSavePuzzles(file);\n")
		out.write('\trpage_file_close(file);\n')
		# out.write('*/\n')
		out.write('}\n')

		out.write('\n')

		out.write('void worldLoadScenarioFlags(rpage_file file)\n')
		out.write('{\n')
		# out.write('/*\n')
		# header
		out.write('\tchar file_header[4];\n')
		# count how many variables
		_var_count = 0
		for var in athanor_con_dict:
			if "type" in athanor_con_dict[var] and athanor_con_dict[var]["type"] == "variable":
				if "save" in athanor_con_dict[var] and athanor_con_dict[var]["save"]:
					_var_count += 1
		out.write('\tUSHORT block_len = ' + str(_var_count) + ';\n')
		out.write('\tUSHORT flag_idx;\n\n')
		out.write("\trpage_file_read(file, &file_header, 4);\n\n")
		out.write('\twhile(block_len--)\n')
		out.write('\t{\n')
		out.write('\t\trpage_file_read(file, &flag_idx, 2);\n')
		out.write('\t\tswitch(flag_idx)\n')
		out.write('\t\t{\n')
		for var in athanor_con_dict:
			if "type" in athanor_con_dict[var] and athanor_con_dict[var]["type"] == "variable":
				if "save" in athanor_con_dict[var] and athanor_con_dict[var]["save"]:
					var_name = athanor_con_dict[var]["name"]
					# out.write("\t/* load variable : " + var_name + " */\n")
					flag_idx = int(''.join(c for c in var_name if c.isdigit()))
					out.write("\t\t\tcase " + str(flag_idx) + ":\n")
					out.write("\t\t\t\trpage_file_read(file, &" + var_name + ", 2);\n")
					out.write("\t\t\t\tbreak;\n")
		out.write('\t\t}\n')
		out.write('\t}\n')
		# out.write('*/\n')
		out.write('}\n')
		out.write('\n')

		out.write('void worldLoadSpritesState(rpage_file file)\n')
		out.write('{\n')
		out.write('\tchar file_header[4];\n')
		out.write('\tshort block_len, go_idx;\n')
		out.write('\tUSHORT us;\n')
		out.write('\tUSHORT b;\n\n')
		out.write("\trpage_file_read(file, &file_header, 4);\n")
		# out.write('\tprintf("%c%c%c%c\\n", file_header[0], file_header[1], file_header[2], file_header[3]);\n')
		out.write("\trpage_file_read(file, &block_len, 2);\n\n")
		# out.write('\tprintf("block_len = %d\\n, ", block_len);\n')
		out.write('\twhile(block_len--)\n')
		out.write('\t{\n')
		out.write("\t\trpage_file_read(file, &us, 2);\n")
		out.write("\t\trpage_file_read(file, &b, 2);\n")
		out.write('\t\tgo_idx = worldGetGameObjectFromCRC16(us);\n')
		# out.write('\t\tprintf("us/b = %x/%x, ", us, b);\n')
		out.write('\t\tworld_sprites[go_to_sprite[go_idx]].enabled = (BOOL)b;\n')
		out.write('\t}\n')
		out.write('}\n')

		out.write('\n')

		out.write('void gameLoadSlot(short slot, BOOL is_on_hdd)\n')
		out.write('{\n')
		# out.write('/*\n')
		out.write('\trpage_file file;\n')
		out.write('\tchar file_header[4];\n')
		out.write('\tchar filename[16];\n')
		out.write('\tif (is_on_hdd)\n')
		out.write('\t\tsprintf(filename, "save.%03d", slot);\n')
		out.write('\telse\n')
		out.write('\t\tsprintf(filename, "%ssave.%03d", SAVE_DISK_NAME, slot);\n')
		out.write('\tfile = rpage_file_open(filename, MODE_OPEN_FILE);\n')
		out.write("\trpage_file_read(file, &file_header, 4);\n")

		out.write("\tgame_load_time(file);\n")
		out.write("\tgameLoadWorldState(file);\n")
		out.write("\tgameLoadInventoryState(file);\n")
		out.write("\tworldLoadScenarioFlags(file);\n")
		out.write("\tworldResetSpritesState();\n")
		out.write("\tworldLoadSpritesState(file);\n")
		out.write("\tgameLoadRoomLinks(file);\n")
		out.write("\tgameLoadPuzzles(file);\n")
		out.write('\trpage_file_close(file);\n')
		# out.write('*/\n')
		out.write('}\n')
		out.write('\n')

		out_h.write('#endif\n')
		out_h.close()
		out.close()
		json_data.close()
