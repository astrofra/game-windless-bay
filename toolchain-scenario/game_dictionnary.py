# search & replace, dictionnary, specific to the game

def athanor_var(variable_name):
    return 'go_' + variable_name


def athanor_sprite(sprite_name):
    return 'spr_' + sprite_name


def untag_digit(_str):
    return '"' + _str.replace('#', 'pal_') + '"'
    # return (int(_str.replace('#', '')))


def vue_index_to_game_part(idx):
    return "iceland"


def replace_athanor_semantic(_str):
    _str = _str.replace("cnosos", "cnossos")
    _str = _str.replace("bateau", "boat")
    if _str.find("rapa") > -1 and _str.find("rapanui") == -1:
        _str = _str.replace("rapa", "rapanui")
    return _str


world_dict = ["blank", "iceland"]
chapter_dict = ["blank", "iceland"]

game_native_text_dict = {
    "%%%": "%s",
    "disquette 1": "disquette",
    "Insert Disk 1": "Insert Disk",
    "Appuyer Espace": "Appuyez sur le bouton de la souris",
    "Press Space": "Press Mouse Button"
}

# Patch !!!
athanor_missing_vars = [
]

athanor_con_dict = {
    "ClicZone": {"name": "click_zone", "type": "variable", "input": "game_object"},
    "ClicPerso": {"name": "click_spr", "type": "variable", "input": "game_object"},
    "Mode": {"name": "game_is_current_action", "type": "function", "input": "game_action"},
    "SacADos": {"name": "game_is_object_in_inventory", "type": "function", "input": "game_object"},
    "ObjetEnMain": {"name": "game_is_object_in_hand", "type": "function", "input": "game_object"},
    "LastVue": {"name": "previous_vue", "type": "variable", "input": "integer"},
    "ArriveeVue": {"name": "previous_vue", "type": "variable", "input": "integer"},

    "Timer1": {"name": "Timer1", "type": "variable", "input": "integer", "explicit_declaration": False, "save": False},
    "Timer2": {"name": "Timer2", "type": "variable", "input": "integer", "explicit_declaration": False, "save": False},

    "Flag001": {"name": "Flag001", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag002": {"name": "Flag002", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag003": {"name": "Flag003", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag004": {"name": "Flag004", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag005": {"name": "Flag005", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag006": {"name": "Flag006", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag007": {"name": "Flag007", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},

    "Flag009": {"name": "Flag009", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True, "default_value":1},

    "Flag011": {"name": "Flag011", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag012": {"name": "Flag012", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},

    "Flag014": {"name": "Flag014", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag015": {"name": "Flag015", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},

    "Flag017": {"name": "Flag017", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag018": {"name": "Flag018", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag019": {"name": "Flag019", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag020": {"name": "Flag020", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag021": {"name": "Flag021", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag022": {"name": "Flag022", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag023": {"name": "Flag023", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},

    "Flag024": {"name": "Flag024", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},

    "Flag028": {"name": "Flag028", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag029": {"name": "Flag029", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},

    "Flag030": {"name": "Flag030", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag031": {"name": "Flag031", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag032": {"name": "Flag032", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},

    "Flag034": {"name": "Flag034", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},

    "Flag036": {"name": "Flag036", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag037": {"name": "Flag037", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},

    "Flag038": {"name": "Flag038", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag039": {"name": "Flag039", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag040": {"name": "Flag040", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag041": {"name": "Flag041", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},
    "Flag042": {"name": "Flag042", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True},

    "Flag068": {"name": "Flag068", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True, "default_value":1},
    "Flag069": {"name": "Flag069", "type": "variable", "input": "integer", "explicit_declaration": True, "save": True}
}

athanor_set_preprocessor_dict = {
    "Dial": {"name": "game_display_dialog_sequence", "type": "function"},
    "PlaySPL": {}
}

athanor_set_dict = {
    "Special": {"name": "start_game_special", "type": "function", "varargs": False},
    "Dial": {"name": "game_display_dialog", "type": "function", "varargs": True},
    "game_display_dialog_sequence": {"name": "game_display_dialog_sequence", "type": "function", "varargs": True, "nominal_op_count": 2},
    "game_display_dialog_sequence_ex": {"name": "game_display_dialog_sequence_ex", "type": "function", "varargs": True, "nominal_op_count": 3},
    "CurrentVue": {"name": "worldSetCurrentRoomByVue", "type": "function", "varargs": False},
    "Vue": {"name": "worldSetCurrentRoomByVue", "type": "function", "varargs": False},
    "ChangeZne": {"name": "worldSetCurrentZone", "type": "function", "param": "click_zone", "varargs": False},
    "Chapter": {"name": "worldSetCurrentChapter", "type": "function", "varargs": False},
    "ObjetEnMain": {"name": "game_set_object_to_hand", "type": "function", "input": "game_object", "varargs": False, "returns": False},
    "SacADos": {"name": "game_set_object_auto_inventory", "type": "function", "input": "game_object", "varargs": False},
    "HideSPR": {"name": "game_hide_sprite", "type": "function", "input": "sprite", "variable_translator": athanor_var, "varargs": False},
    "ShowSPR": {"name": "game_show_sprite", "type": "function", "input": "sprite", "variable_translator": athanor_var, "varargs": False},
    "PlaySPL": {"name": "game_play_sample", "type": "function", "varargs": True},
    "World": {"name": "world_set_current_index", "type": "function", "varargs": False},
    "CloseNord": {"name": "worldCloseExitNorth", "type": "function", "varargs": False,  "nominal_op_count": 1},
    "CloseEst": {"name": "worldCloseExitEast", "type": "function", "varargs": False,  "nominal_op_count": 1},
    "CloseSud": {"name": "worldCloseExitSouth", "type": "function", "varargs": False,  "nominal_op_count": 1},
    "CloseOuest": {"name": "worldCloseExitWest", "type": "function", "varargs": False,  "nominal_op_count": 1},
    "OpenNord": {"name": "worldOpenExitNorth", "type": "function", "varargs": False,  "nominal_op_count": 2},
    "OpenEst": {"name": "worldOpenExitEast", "type": "function", "varargs": False,  "nominal_op_count": 2},
    "OpenSud": {"name": "worldOpenExitSouth", "type": "function", "varargs": False,  "nominal_op_count": 2},
    "OpenOuest": {"name": "worldOpenExitWest", "type": "function", "varargs": False,  "nominal_op_count": 2},
    "AbortDepart": {"name": "game_abort_leaving_room", "type": "function", "varargs": False},
    "Tempo": {"name": "game_wait_ticks", "type": "function", "varargs": False},
    "FadeOUTPalette": {"name": "game_fade_out", "type": "function", "varargs": False},
    "FadeINPalette": {"name": "game_fade_in", "type": "function", "varargs": False},
    "GameOver": {"name": "game_over", "type": "function", "varargs": False},
    "PlayZik": {"name": "game_load_music", "type": "function", "varargs": False, "collect_to_table": "music_symbols", "collect_prefix": "mus_"},
    "StopZik": {"name": "game_stop_music", "type": "function", "varargs": False },
    # "PlayZik": {"name": "game_play_music", "type": "function", "varargs": False},
    "SetTimer1": {"name": "game_enable_timer1", "type": "function", "varargs": False},
    "SetTimer2": {"name": "game_enable_timer2", "type": "function", "varargs": False},
    "SetPalette": {"name": "game_fadeto_palette", "type": "function", "varargs": False, "variable_translator": untag_digit},
}

# enum game_action { act_take_drop, act_look, act_use, act_talk, act_save, act_load };

game_action_dict = {
    "TAKE": "act_take_drop",
    "LOOK": "act_look",
    "USE": "act_use",
    "TALK": "act_talk"
}

game_cardinal_order = ["north", "south", "east", "west"]

vue_short_names = [
    "empty", #0
    "rapa_beach", #1
    "rapa_boat_wreck",
    "rapa_boat_wreck_front",
    "rapa_boat_wreck_stairs",
    "rapa_fisherman", #5
    "rapa_moai_far",
    "rapa_moai_close",
    "rapa_valley",
    "rapa_cliff_top",
    "rapa_stone_mask", #10
    "rapa_cliff_shore",
    "rapa_cliff_shore_statue",
    "rapa_XXX",
    "rapa_YYY",
    "rapa_cavern_entrance", #15
    "rapa_giant_squid",
    "rapa_atlantis_ship",
    "rapa_atlantis_ship_flying",
    "",
    "",
    "",
    "",
    "",
    "",
    "cnossos_boat", #25
    "cnossos_mosque",
    "cnossos_fishermen",
    "cnossos_drying_fish",
    "cnossos_street_0",
    "cnossos_street_1", #30
    "cnossos_wine_shop",
    "cnossos_square_statue",
    "cnossos_square_tree",
    "cnossos_inn_entrance",
    "cnossos_inn", #35
    "cnossos_street_2",
    "cnossos_guarded_door",
    "cnossos_altos_house_0",
    "cnossos_altos_house_1",
    "cnossos_pedestal_room", #40
    "cnossos_end_of_village",
    "cnossos_lilla_bedroom",
    "",
    "",
    "indus_boat", #45
    "indus_path_0",
    "indus_tent",
    "indus_path_1",
    "indus_village_0",
    "indus_house_0", #50
    "indus_potter",
    "indus_river_valley",
    "indus_village_1",
    "indus_temple_outside",
    "indus_temple_entrance", #55
    "indus_temple_hermite",
    "indus_path_2",
    "indus_village_2",
    "indus_village_race_game",
    "indus_village_cows", #60
    "indus_temple_inside",
    "",
    "",
    "",
    "", #65
    "",
    "",
    "",
    "",
    "", #70
    "boat_bridge",
    "boat_steering_wheel",
    "boat_cabin",
    "boat_storage"
]