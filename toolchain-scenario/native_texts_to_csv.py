# Athanor native files converter

import os
import shutil
import json
import pyphen
from translate import Translator

# folder_tmp = "_temp_/"
# assets_path = "../../game/assets/"
resources_out_path = "../../toolchain-resources/resources/"
locale_ext = ['fr'] # , 'en', 'es']
dialogs_path = "resources/DIALS"
dialog_files = ["BATEAU.TXT", "CNOSSOS.TXT", "INDUS.TXT", "RAPA.TXT", "CREDITS.TXT"]
world_rooms = [{'min': 1, 'max': 24, 'world_name': 'rapanui'}, {'min': 25, 'max': 42, 'world_name': 'cnossos'},
			   {'min': 45, 'max': 61, 'world_name': 'indus'}, {'min': 71, 'max': 72, 'world_name': 'boat'}]
system_files = ["SYS.TXT"]
tooltip_files = ["BULLE.TXT"]

for dialog_file in dialog_files:
	with open(os.path.join(dialogs_path, locale_ext[0], dialog_file), encoding='cp437') as dialog_raw:
		dialog_raw_str = dialog_raw.readlines()
		cleaned_buffer = ''
		for raw_line in dialog_raw_str:
			# raw_line = raw_line.strip()
			raw_line = raw_line.replace('$', '')
			raw_line = raw_line.replace('\n', ' ')
			raw_line = raw_line.replace('  ', ' ')
			if raw_line is not None:
				print(raw_line)
				if raw_line.startswith('#'):
					raw_line = raw_line[0:6] + '\t;\t' + raw_line[6:]
					cleaned_buffer += '\n' + raw_line
				else:
					cleaned_buffer += raw_line
		cleaned_buffer = cleaned_buffer[1:]

		with open(os.path.join(dialogs_path, locale_ext[0], dialog_file + '.csv'), 'w', encoding='utf8') as outfile:
			outfile.write(cleaned_buffer)