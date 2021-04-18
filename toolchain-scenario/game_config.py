# Athanor Scenario toolchain
# Config variables (paths)

# scenario
# output c files
out_c_file = "../game/game/world.c"
out_h_file = "../game/game/world_const.h"

# click zones
zone_native_files = ["resources/SCENAR/iceland.ZNE"]

# list of game objects that can be combined (A + B -> C)
game_object_combined = ["resources/SCENAR/COMBI.TXT"]

# Safar Script scenario
condition_script_native_files = ["resources/SCENAR/CHAP1.CON"]

# Game Sprites Sheets descriptions (json)
sprites_coord_files = ["resources/sprites/spr_iceland.json"]

# NPC Sheets descriptions (json)
sprites_portraits = "resources/sprites/spr_portraits.json"

sprites_inventory_file = "resources/sprites/spr_inventory.json"

gfa_listing_file = "resources/LEGEND10.LST"

resources_out_path = "../toolchain-resources/resources/"

enum_link_direction = ["north", "east", "south", "west"]

# disk_dispatch_file = "../toolchain-resources/resources/dispatch.json"

# dialogs
dialogs_path = "resources/DIALS"

dialog_files = ["iceland.TXT"]

world_rooms = [{'min': 1, 'max': 00, 'world_name': 'iceland'}]

system_files = ["SYS.TXT"] # , "AMI.TXT"]

tooltip_files = ["BULLE.TXT"]

credits_files = ["CREDITS.TXT", "CREDITS_AMIGA.TXT"]  # the order of the files matters!

FILENAME_OBFUSCATION_ENABLED = False
MAX_ZONE_PER_ROOM = 8
MAX_FILENAME_CHAR_LEN = 64
# MAX_INTERACTION_PER_OBJECT = 4
FILENAME_SALT = "CrueltyhasahumanheartAndJealousyahumanface"

# Replace enums by #defines (reduce binary size on the target platform)
enum_instead_of_defines = False
