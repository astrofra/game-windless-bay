# Athanor native files converter

import os
import json
import pyphen
from game_config import *
from game_dictionnary import *

# folder_tmp = "_temp_/"
# assets_path = "../game/assets/"
locale_ext = ['fr', 'en']
# locale_ext = ['fr']

FONT_GLYPH_PIXEL_WIDTH = 6
WINDOW_PIXEL_WIDTH = 256 - FONT_GLYPH_PIXEL_WIDTH


def arr_assign(arr, key, val, default_value = None):
	try:
		arr[key] = val
		return
	except IndexError:
		# Do not extend the array for negative indices
		# That is ridiculously counterintuitive
		assert key >= 0
		arr.extend(((key + 1) - len(arr)) * [default_value])
		arr[key] = val
		return


def preprocess_native_string(_str):
	if game_native_text_dict is not None and len(game_native_text_dict) > 0:
		for replace_str in game_native_text_dict:
			if _str.find(replace_str) > -1:
				_str = _str.replace(replace_str, game_native_text_dict[replace_str])

	return _str


def beautify_string(_str):
	_new_str = ''
	# for _i in range(len(_str)):
	# 	c = _str[_i]
	# 	_new_str += c
	# 	if _i < len(_str) - 1 and c == '.' and _str[_i + 1] != ' ':
	# 		_new_str += ' '
	# 	elif _i < len(_str) - 1 and c == ',' and _str[_i + 1] != ' ':
	# 		_new_str += ' '
	# 	elif _i < len(_str) - 1 and c == '!' and _str[_i + 1] != ' ':
	# 		_new_str += ' '
	_new_str = _str.strip()

	_new_str = _new_str.replace('"', "'")

	if _new_str.endswith('$') > -1:
		_new_str = _new_str.replace('$', '')

	# cleanup
	# remove any duplicate space
	while _str.find('  ') > -1:
		_str = _str.replace('  ', ' ')

	# remove any space preceding a punctuation mark
	for c in get_punctuation_list():
		_str = _str.replace(' ' + c, c)

	# fix any missing space
	for c in ["!", ".", ",", ";", ".", ":"]:
		i = _str.find(c)
		if (i > -1) and (i < len(_str) - 1) and (_str[i + 1] != ' '):
			_str = _str[0:i + 1] + ' ' + _str[i + 1:-1]

	if not (_new_str.endswith('.') or _new_str.endswith('!') or _new_str.endswith('?')):
		_new_str += '.'

	_new_str = _new_str.replace('. . .', '...')
	_new_str = _new_str.replace('-\n', '')

	# if _new_str[-1:] != '.' and _new_str[-1:] != '?' and _new_str[-1:] != '!':
	# 	_new_str += '.'

	return _new_str


def get_punctuation_list():
	return ["!", ".", ",", ";", ".", ":", "?", "-", "_"]


def hyphen_paragraph(_str, dic):
	if _str.find('Prendre ce tentacule sera votre') > -1:
		print("!!!")

	# cleanup patch!
	_str = _str.replace('£', ' ')

	# remove the trailing CR
	if _str.endswith('.[CR].'):
		_str = _str[:-6] + '.'

	if _str.endswith('[CR].'):
		_str = _str[:-5] + '.'

	# search for legit CR
	_dialog = _str
	_new_dialog = ''
	_splt = _dialog.split('[CR] ')
	for i, _str in enumerate(_splt):
		if i < len(_splt) - 1:
			if _splt[i + 1][0].isalpha() and _splt[i + 1][0] == _splt[i + 1][0].upper():
				_new_dialog += _str + '\\n'
			else:
				_new_dialog += _str + '[CR] '
		else:
			_new_dialog += _str

	_str = _new_dialog

	_str = _str.replace('[CR] ', '[CR]')
	_str = _str.replace('.[CR]', '.\\n')
	_str = _str.replace('[CR]', ' ')
	_str = _str.replace('...', '[***]')
	_str = _str.replace('..', '.')
	_str = _str.replace('[***]', '...')

	_dialog = _str
	_new_dialog = ''
	for _str in _dialog.split('\\n'):
		word_list = _str.split(' ')
		line_char_len = WINDOW_PIXEL_WIDTH // FONT_GLYPH_PIXEL_WIDTH
		c_x = 0
		_str = ''
		for current_word, word in enumerate(word_list):
			if c_x + len(word) + 1 > line_char_len:
				hyphen_len = 1
				best_hyphen = -1
				sub_word = None
				while hyphen_len < len(word):
					sub_word = dic.wrap(word, hyphen_len)
					hyphen_len += 1
					if sub_word is not None:
						if c_x + len(sub_word[0]) + 1 < line_char_len:
							best_hyphen = hyphen_len
						else:
							# sub_word = dic.wrap(word, best_hyphen)
							break

				if best_hyphen > 0:
					sub_word = dic.wrap(word, best_hyphen)

				if sub_word is None or c_x + len(sub_word[0]) + 1 > line_char_len:
					_str += '\\n'
					_str += word
					c_x = len(word)
				else:
					if _str[:-1] != ' ':
						_str += ' '
					_str += sub_word[0] + '\\n'
					_str += sub_word[1]
					c_x = len(sub_word[1]) + 1
			else:
				if c_x > 0:
					_str += ' '
					c_x += 1

				_str += word
				c_x += len(word)

		# print('-' * (line_char_len))
		# print(_str.replace('\\n', "\n"))
		_new_dialog += _str + '\\n'

	if _new_dialog.endswith('\\n'):
		_new_dialog = _new_dialog[:-2]
	if _new_dialog.endswith('!.'):
		_new_dialog = _new_dialog[:-1]
	if _new_dialog.endswith('?.'):
		_new_dialog = _new_dialog[:-1]
	while _new_dialog.find('  ') > -1:
		_new_dialog = _new_dialog.replace('  ', ' ')
	_new_dialog = _new_dialog.replace('--', '-')
	_new_dialog = _new_dialog.replace('\\n?\\n', '?\\n')
	_new_dialog = _new_dialog.replace('\\n!\\n', '!\\n')
	_new_dialog = _new_dialog.replace('\\n.\\n', '.\\n')
	_new_dialog = _new_dialog.replace('\\n,\\n', ',\\n')
	_new_dialog = _new_dialog.replace('\\n;\\n', ';\\n')
	_new_dialog = _new_dialog.replace('\\n:\\n', ':\\n')
	_new_dialog = _new_dialog.replace('..\\n.', '...')
	if _new_dialog.endswith('\\n?'):
		_new_dialog = _new_dialog[:-3] + '?'

	return _new_dialog


def amiga_str_len(str):
	while str.find('\\') > -1:
		str = str.replace("\\", '')
	return len(str)


def write_amiga_encoded_string(outfile, str):
	encoding = 'latin1'
	l = str.split("\\n")
	for i, s in enumerate(l):
		l2 = s.split("\\'")
		for j, s2 in enumerate(l2):
			outfile.write(s2.encode(encoding))
			if str.find("\\'") > -1 and j < len(l2) - 1:
				outfile.write("'".encode(encoding))
		if str.find("\\n") > -1 and i < len(l) - 1:
			outfile.write((0x0A).to_bytes(1, byteorder='big', signed=False))

	# return bstr


for language in locale_ext:
	print("Processing binary export in " + language + " language!")
	if language.lower() == 'fr':
		dic = pyphen.Pyphen(lang='fr_FR', left=1)
	else:
		dic = pyphen.Pyphen(lang='en_GB')

	room_max = 0
	dialog_max = 0
	dialog_per_room_max = 0
	merged_files = []
	merged_system_files = []
	dialogs = []
	sys_dialogs = []
	tooltip_dialogs = []
	credits_dialogs = []

	#
	# Collect the .Txt files
	#
	for dialog_file in dialog_files:
		print("------------------------------------------------------------------")
		print("--------- PROCESSING '" + dialog_file + "' -----------------------")
		print("------------------------------------------------------------------")
		with open(os.path.join(dialogs_path, language, dialog_file), encoding='cp437') as dialog_raw:
			dialog_raw_str = dialog_raw.readlines()
			for raw_line in dialog_raw_str:
				raw_line = raw_line.strip(' ')
				raw_line = raw_line.replace('\n', '[CR]')
				if raw_line is not None and raw_line.find("#0000") < 0:
					raw_line = raw_line.replace("\"", "\\\"")
					raw_line = raw_line.replace("\'", "\\\'")
					merged_files.append(raw_line)
					# if not (raw_line in merged_files):
					# 	# if raw_line.find('Prendre ce tentacule sera votre') > -1:
					# 	# 	print("!!!!")
					# 	merged_files.append(raw_line)
					# else:
					# 	print("found dup: " + raw_line)

	print(len(merged_files))

	for dialog_file in system_files:
		print("------------------------------------------------------------------")
		print("--------- PROCESSING '" + dialog_file + "' -----------------------")
		print("------------------------------------------------------------------")
		with open(os.path.join(dialogs_path, language, dialog_file), encoding='cp437') as dialog_raw:
			dialog_raw_str = dialog_raw.readlines()
			for raw_line in dialog_raw_str:
				raw_line = raw_line.strip()
				if raw_line is not None and raw_line.find("#0000") < 0:
					raw_line = raw_line.replace("\"", "\\\"")
					raw_line = raw_line.replace("\'", "\\\'")
					merged_system_files.append(raw_line)
					# if not (raw_line in merged_system_files):
					# 	merged_system_files.append(raw_line)
					# else:
					# 	print("found dup: " + raw_line)

	print(len(merged_system_files))

	for tooltip_file in tooltip_files:
		print("------------------------------------------------------------------")
		print("--------- PROCESSING '" + tooltip_file + "' -----------------------")
		print("------------------------------------------------------------------")
		with open(os.path.join(dialogs_path, language, tooltip_file), encoding='cp437') as dialog_raw:
			dialog_raw_str = dialog_raw.readlines()
			for raw_line in dialog_raw_str:
				raw_line = raw_line.strip()
				if raw_line is not None:
					tooltip_dialogs.append(raw_line)

	print(len(tooltip_dialogs))

	for credit_file in credits_files:
		print("------------------------------------------------------------------")
		print("--------- PROCESSING '" + credit_file + "' -----------------------")
		print("------------------------------------------------------------------")
		with open(os.path.join(dialogs_path, language, credit_file), encoding='cp437') as dialog_raw:
			dialog_raw_str = dialog_raw.readlines()
			for raw_line in dialog_raw_str:
				raw_line = raw_line.strip()
				raw_line = raw_line.replace('@', ' ')
				raw_line = raw_line.replace('Musiks', 'Musics')
				if raw_line is not None and len(raw_line) > 0:
					if raw_line[0] == '#':
						credits_dialogs.append(' ')
						raw_line = raw_line[1:]
					credits_dialogs.append(raw_line)

	print(len(credits_dialogs))

	def marker_is_end_of_file(_str):
		for c in _str.strip():
			if c != '#' and c != '0':
				return False
		return True


	def marker_is_points_only(_str):
		for c in _str.strip():
			if c != '.':
				return False
		return True

	#
	# Process the collected texts
	#
	dialog_index = -1
	linear_dialog_index = 0
	room = -1
	dialog_str = ""
	tmp_room_dialogs = []
	dynamic_dialogs = []

	for line_idx, raw_line in enumerate(merged_files):
		# print(dialog_line)
		mode = raw_line[0]
		print(tmp_room_dialogs)
		if not marker_is_end_of_file(raw_line) and not marker_is_points_only(raw_line):
			if mode == '#':
				# record previous dialog (if any)
				tmp_room_dialogs = []
				if dialog_index != -1:
					if room < len(dialogs):
						tmp_room_dialogs = dialogs[room].copy()
					dialog_str = preprocess_native_string(dialog_str)
					dialog_str = beautify_string(dialog_str) # hyphen_paragraph(beautify_string(dialog_str))
					arr_assign(tmp_room_dialogs, dialog_index, dialog_str)
					arr_assign(dialogs, room, tmp_room_dialogs.copy(), [])

					dialog_max += 1

					dialog_index = -1

				# get dialog string
				dialog_str = raw_line[6:] + ' '
				# dialog_str = beautify_string(dialog_str)
				# dialog_str = hyphen_paragraph(dialog_str)

				# get room index
				room = raw_line[1:4]
				room = int(room)
				room_max = max(room_max, room)

				# get dialog index
				dialog_index = int(raw_line[4:6])
			else:
				# dialog_str += hyphen_paragraph(beautify_string(raw_line))
				# dialog_str += beautify_string(raw_line)
				dialog_str += raw_line + ' '

	room_max += 1

	# dialogs = sorted(dialogs.items(), key=lambda t: t[0])
	print(json.dumps(dialogs))

	for raw_line in merged_system_files:
		if raw_line.lower().startswith("sys"):
			sys_index = int(raw_line[3:6])
			sys_txt = raw_line[6:]
			sys_txt = preprocess_native_string(sys_txt)
			sys_dialogs.append({"index": sys_index, "dialog": sys_txt})
		else:
			if raw_line.lower() == "vide":
				sys_dialogs.append({"index": 0, "dialog": ""})

	print(json.dumps(sys_dialogs))

	for room_index, room in enumerate(dialogs):
		dialog_per_room_max = max(dialog_per_room_max, len(room))

	#
	# Generate the .c/.h files
	# we do it for the french only
	#

	dynamic_string_max_len = 0

	if language == 'fr':
		print("room_max = " + str(room_max))
		print("dialog_per_room_max = " + str(dialog_per_room_max))

		with open('../game/game/text.c', 'w', encoding='latin1') as outfile:
			outfile.write('#include "game/text.h"\n\n')

			outfile.write('/* THIS CODE WAS GENERATED */\n')
			outfile.write('/*    DON\'T MODIFY IT !    */\n\n')

			# build and write the index of dialogues, stored by rooms
			outfile.write('short dialog_per_room[GAME_MAX_ROOM][GAME_MAX_DIALOG_PER_ROOM] =\n')
			outfile.write('{\n')
			linear_dialog_index = 0
			for room_index, room in enumerate(dialogs):
				dialog_indexes = [-1] * dialog_per_room_max
				if len(room) > 0:
					for dialog_index, dialog in enumerate(room):
						if dialog is not None:
							if dialog.find("%s") > -1:
								dynamic_dialogs.append(linear_dialog_index)
								dynamic_string_max_len = max(dynamic_string_max_len, len(dialog))
							dialog_indexes[dialog_index] = linear_dialog_index
							linear_dialog_index += 1
				outfile.write('\t{')
				for dialog_index in dialog_indexes:
					outfile.write(str(dialog_index) + ',')
				outfile.write('}, \t\t/* vue_' + str(room_index) + ' */\n')

			outfile.write('};\n')

			outfile.write('\n')

			outfile.write('/* DEBUG ONLY */\n')

			# dic = pyphen.Pyphen(lang='fr_FR')

			# write the linear table of dialogs
			linear_dialog_index = 0
			outfile.write('/* char *dialogs[GAME_MAX_DIALOG] =\n')
			outfile.write('{ */\n')
			for room_index, room in enumerate(dialogs):
				# outfile.write('{\n')
				if len(room) > 0:
					for dialog_index, dialog in enumerate(room):
						if dialog is not None:
							dialog = hyphen_paragraph(dialog, dic)
							outfile.write('\t/* ' + str(linear_dialog_index) + ' "' + dialog + '",' + ' */\n')
							# print(str(linear_dialog_index) + " " + str(room_index) + " " + str(dialog_index) + " " + dialog)
							linear_dialog_index += 1
			# 		else:
			# 			outfile.write('\t"\\0",\n')

			outfile.write('/* };\n')
			outfile.write('*/\n')

			outfile.write('\n')

			# write the linear table of system dialogs
			outfile.write('/* System dialogs */\n')

			linear_dialog_index = 0
			outfile.write('/* char *system_dialogs[SYS_MAX_DIALOG] =\n')
			outfile.write('{ */\n')
			for dialog in sys_dialogs:
				outfile.write('\t/* SYS{:02d} "{}", */\n'.format(dialog["index"], hyphen_paragraph(dialog["dialog"], dic)))
				# outfile.write('\t"' + dialog["dialog"] + ('", /* SYS{:02d}'.format(dialog["index"])) + ' */ \n')
			outfile.write('/* }; */\n')

			outfile.write('\n')

			# write the objects name tooltips
			outfile.write('/* Inventory Tooltip Index */\n')

			outfile.write('/* char *tooltip_dialogs[TOOLTIP_MAX_DIALOG] =\n')
			outfile.write('{ /*\n')
			for index, dialog in enumerate(tooltip_dialogs):
				outfile.write('\t/* {:02d} "{}", */\n'.format(index, dialog))
			# outfile.write('\t"' + dialog["dialog"] + ('", /* SYS{:02d}'.format(dialog["index"])) + ' */ \n')
			outfile.write('/* }; */\n')

			outfile.write('\n')

			# write the credits
			outfile.write('/* Credits */\n')

			outfile.write('/* char *credits_dialogs[CREDITS_MAX_DIALOG] =\n')
			outfile.write('{ /*\n')
			for index, dialog in enumerate(credits_dialogs):
				outfile.write('\t/* {:02d} "{}", */\n'.format(index, dialog))
			outfile.write('/* }; */\n')
			# Indexes of the dynamic dialogs ("%s")
			outfile.write('\n/* Dynamic dialogs indexes */\n')
			outfile.write('short dynamic_dialogs[GAME_MAX_DYNAMIC_DIALOG] = {')
			_d_str = ''
			for d in dynamic_dialogs:
				_d_str += str(d) + ', '
			_d_str = _d_str[:-2]
			outfile.write(_d_str + '};\n')

		dynamic_string_max_len = (int(dynamic_string_max_len * 1.5) // 16) * 16

		with open('../game/game/text.h', 'w', encoding='latin1') as outfile:
			outfile.write('#ifndef _GAME_DATA_\n')
			outfile.write('#define _GAME_DATA_\n\n')

			outfile.write('/* THIS CODE WAS GENERATED */\n')
			outfile.write('/*    DON\'T MODIFY IT !    */\n\n')

			outfile.write('#define GAME_MAX_ROOM ' + str(room_max) + '\n')
			outfile.write('#define GAME_MAX_DIALOG_PER_ROOM ' + str(dialog_per_room_max) + '\n')
			outfile.write('#define GAME_MAX_DIALOG ' + str(dialog_max) + '\n')
			outfile.write('#define GAME_MAX_DYNAMIC_DIALOG ' + str(len(dynamic_dialogs)) + '\n')
			outfile.write('#define GAME_DYNAMIC_DIALOG_MAX_LEN ' + str(dynamic_string_max_len) + '\n\n')
			outfile.write('#define SYS_MAX_DIALOG ' + str(len(sys_dialogs)) + '\n')
			outfile.write('#define TOOLTIP_MAX_DIALOG ' + str(len(tooltip_dialogs)) + '\n')
			outfile.write('#define CREDITS_MAX_DIALOG ' + str(len(credits_dialogs)) + '\n\n')

			outfile.write('extern short dialog_per_room[GAME_MAX_ROOM][GAME_MAX_DIALOG_PER_ROOM];\n')
			outfile.write('extern char *dialogs[GAME_MAX_DIALOG];\n')
			outfile.write('extern char *system_dialogs[SYS_MAX_DIALOG];\n')
			outfile.write('extern char *tooltip_dialogs[TOOLTIP_MAX_DIALOG];\n')
			outfile.write('extern char *credits_dialogs[CREDITS_MAX_DIALOG];\n')
			outfile.write('extern short dynamic_dialogs[GAME_MAX_DYNAMIC_DIALOG];\n')

			outfile.write('\n')

			for dialog in sys_dialogs:
				outfile.write('#define SYS{:02d} (-{:d})\n'.format(dialog["index"], dialog["index"]))


			outfile.write('#endif\n')

	#
	# Generate the .pak files (packed binary files)
	#

	# if os.path.exists(folder_tmp):
	# 	shutil.rmtree(folder_tmp)
	# os.mkdir(folder_tmp)

# native_locale = 'fr'
	print(" -> Dialogs")
	with open(os.path.join(resources_out_path, 'dialog_' + language + '.bintext'), 'wb') as outfile:
		# write the linear table of dialogs
		linear_dialog_index = 0
		for room_index, room in enumerate(dialogs):
			# outfile.write('{\n')
			if len(room) > 0:
				for dialog_index, dialog in enumerate(room):
					if dialog is not None:
						# if language != native_locale:
						# 	translator = Translator(from_lang="fr", to_lang=language)
						# 	dialog = translator.translate(dialog)
						dialog = hyphen_paragraph(dialog, dic)
						outfile.write((amiga_str_len(dialog) + 1).to_bytes(2, byteorder='big', signed=False))
						# outfile.write(str(dialog).encode('latin1'))
						write_amiga_encoded_string(outfile, dialog)
						outfile.write(b'\0')
						# print(str(linear_dialog_index) + " " + str(room_index) + " " + str(dialog_index) + " " + dialog)
						linear_dialog_index += 1

	print(" -> System messages")
	with open(os.path.join(resources_out_path, 'system_' + language + '.bintext'), 'wb') as outfile:
		# write the linear table of dialogs
		linear_dialog_index = 0
		for dialog_entry in sys_dialogs:
			dialog = dialog_entry["dialog"]
			# if language != native_locale:
			# 	translator = Translator(from_lang="fr", to_lang=language)
			# 	dialog = translator.translate(dialog)
			dialog = hyphen_paragraph(dialog, dic)
			outfile.write((amiga_str_len(dialog) + 1).to_bytes(2, byteorder='big', signed=False))
			# outfile.write(str(dialog).encode('latin1'))
			write_amiga_encoded_string(outfile, dialog)
			outfile.write(b'\0')

	print(" -> Inventory tooltip")
	with open(os.path.join(resources_out_path, 'tooltip_' + language + '.bintext'), 'wb') as outfile:
		# write the linear table of dialogs
		linear_dialog_index = 0
		for dialog in tooltip_dialogs:
			# if language != native_locale:
			# 	translator = Translator(from_lang="fr", to_lang=language)
			# 	dialog = translator.translate(dialog)
			outfile.write((amiga_str_len(dialog) + 1).to_bytes(2, byteorder='big', signed=False))
			# outfile.write(str(dialog).encode('latin1'))
			write_amiga_encoded_string(outfile, dialog)
			outfile.write(b'\0')

	print(" -> Game Credits")
	with open(os.path.join(resources_out_path, 'credits_' + language + '.bintext'), 'wb') as outfile:
		linear_dialog_index = 0
		for dialog in credits_dialogs:
			outfile.write((amiga_str_len(dialog) + 1).to_bytes(2, byteorder='big', signed=False))
			write_amiga_encoded_string(outfile, dialog)
			outfile.write(b'\0')

#	c_outfile.write('TXPK'.encode())


# json.dump(dialogs, outfile, sort_keys=True, indent=4)