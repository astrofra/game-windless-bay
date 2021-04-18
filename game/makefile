.prl.gst:
	sc NOLINK STRIPDEBUG CODE=N DATA=F MGST $*.gst $*.prl

.c.o:
	sc NOLINK STRIPDEBUG CODE=N DATA=F TO $@ $*.c

OBJ = main.o \
rpage/aos/frwk.o \
rpage/aos/screen.o rpage/aos/color.o rpage/aos/bitmap.o rpage/aos/locale.o\
rpage/aos/helper.o rpage/aos/ptracker.o rpage/aos/sound.o\
ext/aos/shrinkler.o rpage/aos/time.o ext/tinfl.o ext/aos/nrv2.o\
rpage/utils.o rpage/aos/io.o rpage/aos/mouse_ptr.o \
rpage/easing.o rpage/err.o \
game/vue.o game/game.o game/gui.o game/gui_tile.o game/world.o game/text.o \
game/special.o game/aos/files.o game/aos/assets.o game/aos/sprites.o

main.exe: $(OBJ) includes.gst
	sc LINK STRIPDEBUG CODE=N DATA=F OBJ $(OBJ) TO main.exe

includes.gst: rpage/aos/inc.prl
main.o: main.c rpage/frwk.h rpage/aos/screen_size.h rpage/aos/screen.h rpage/aos/bitmap.h\
rpage/aos/helper.h rpage/aos/color.h rpage/aos/ptracker.h rpage/aos/debug.h ext/aos/shrinkler.h\
rpage/aos/io.h rpage/utils.h rpage/aos/mouse_ptr.h rpage/aos/time.h ext/tinfl.h rpage/aos/sound.h rpage/aos/locale.h\
game/vue.h game/game.h game/world.h game/special.h rpage/easing.h game/aos/assets.c \
game/world_const.h game/text.h includes.gst
frwk.o: rpage/aos/frwk.c rpage/frwk.h
screen.o: rpage/aos/screen.c
color.o: rpage/aos/color.c
bitmap.o: rpage/aos/bitmap.c
helper.o: rpage/aos/helper.c
ptracker.o: rpage/aos/ptracker.c rpage/aos/ptracker.h
sound.o: rpage/aos/sound.c rpage/aos/sound.h rpage/aos/adpcm.h
io.o: rpage/aos/io.c
time.o: rpage/aos/time.c
locale.o: rpage/aos/locale.c rpage/aos/locale.h
mouse_ptr.o: rpage/aos/mouse_ptr.c
easing.o: rpage/easing.c rpage/easing.h
game.o: game/game.c game/game.h
special.o: game/special.c game/special.h game/game.h
gui.o: game/gui.c game/gui.h
gui_tile.o: game/gui_tile.c
vue.o: game/vue.c game/vue.h game/aos/config.h
world.o: game/world.c game/world.h game/special.h
text.o: game/text.c game/text.h
files.o: game/aos/files.c game/aos/files.h
assets.o: game/aos/assetc.c game/aos/assets.h
sprites.o: game/aos/sprites.c game/aos/sprites.h
utils.o: rpage/utils.c
debug.o: rpage/aos/debug.c
shrinkler.o: ext/aos/shrinkler.asm ext/aos/shrinkler.h
nrv2.o: ext/aos/nrv2.asm ext/aos/nrv2.h
tinfl.o: ext/tinfl.c ext/tinfl.h
err.o: rpage/err.c rpage/err.h