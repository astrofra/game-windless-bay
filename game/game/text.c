#include "game/text.h"

/* THIS CODE WAS GENERATED */
/*    DON'T MODIFY IT !    */

short dialog_per_room[GAME_MAX_ROOM][GAME_MAX_DIALOG_PER_ROOM] =
{
	{-1,-1,}, 		/* vue_0 */
	{0,-1,}, 		/* vue_1 */
	{1,2,}, 		/* vue_2 */
	{3,-1,}, 		/* vue_3 */
	{4,-1,}, 		/* vue_4 */
	{5,-1,}, 		/* vue_5 */
	{6,-1,}, 		/* vue_6 */
	{7,-1,}, 		/* vue_7 */
	{8,-1,}, 		/* vue_8 */
	{9,-1,}, 		/* vue_9 */
	{10,-1,}, 		/* vue_10 */
	{11,-1,}, 		/* vue_11 */
	{12,-1,}, 		/* vue_12 */
	{13,-1,}, 		/* vue_13 */
	{14,-1,}, 		/* vue_14 */
	{15,-1,}, 		/* vue_15 */
	{16,-1,}, 		/* vue_16 */
	{17,-1,}, 		/* vue_17 */
	{18,-1,}, 		/* vue_18 */
};

/* DEBUG ONLY */
/* char *dialogs[GAME_MAX_DIALOG] =
{ */
	/* 0 "Le pupitre de la station d\'�coute.\nD\'ici cr�pitent les ondes du monde en-\ntier.", */
	/* 1 "Vous �tes dans le local technique.\nOn y trouve un peu de tout.", */
	/* 2 "Le code de d�v�rouillage est le %s.", */
	/* 3 "Au pied de la tour radio, le froid �\nl\'exterieur est difficilement suppor-\ntable.", */
	/* 4 "La lande � perte de vue.\nDerri�re les montagnes...", */
	/* 5 "Vous �tes au bord du monde, un puit natu-\nrel crache de la vapeur sans disconti-\nnuer.", */
	/* 6 "Une autre station radio? Ou bien est-ce\nune habitation?", */
	/* 7 "A l\'interieur tout est vide.\nUne porte semble mener au sous-sol.", */
	/* 8 "Le sous-sol �tait en r�alit� un gigan-\ntesque puit d\'aeration.", */
	/* 9 "Un long tunnel porte quelques traces de.", */
	/* 10 "Une lourde porte semble mener vers l\'ex-\nterieur.\npassage relativement r�centes.", */
	/* 11 "Une splendide antenne radar vous observe\ndans un silence absolu.", */
	/* 12 "Vous longer un canal vide et d�sol�, une\nrumeur maritime vous parvient depuis le\nsud.", */
	/* 13 "Quelqu\'un �tait ici il y a peu. Un feu\nimprovis� vous rechauffe les mains.", */
	/* 14 "Le port, d�sert, mais les installations\nsont encore en place !", */
	/* 15 "Sur les quais, les entr�es maritimes vous\nappellent.", */
	/* 16 "En haute mer, a des lieues de la c�t�,\nvous accostez sur une station maritime!", */
	/* 17 "Le hangar � bateaux, dans un style digne\ndes constructions Eiffel!", */
	/* 18 "Un petit salon, le cadre est accueillant.", */
/* };
*/

/* System dialogs */
/* char *system_dialogs[SYS_MAX_DIALOG] =
{ */
	/* SYS00 "", */
	/* SYS01 "C\'est dans la poche.", */
	/* SYS02 "Je ne vois rien d\'int�ressant ici!", */
	/* SYS03 "J\'ai beau scruter, il n\'y a rien.", */
	/* SYS04 "Rien de notable.", */
	/* SYS05 "Ne perdez pas de temps.", */
	/* SYS06 "Laissez-moi tranquille, je n\'ai rien �\nvous dire.", */
	/* SYS07 "Je vous ai tout dit.", */
	/* SYS08 "Je ne peux pas faire �a.", */
	/* SYS09 "Vous n\'utilisez pas le bon objet!", */
	/* SYS10 "�a ne donne rien.", */
	/* SYS11 "Vous avez d�j� ce qu\'il vous faut!", */
	/* SYS12 "Charger partie", */
	/* SYS13 "Sauver partie", */
	/* SYS14 "Partie sauvegard�e!", */
	/* SYS15 "Ins�rez disquette", */
	/* SYS16 "Ins�rez disquette 2", */
	/* SYS17 "Appuyez sur le bouton de la souris", */
	/* SYS18 "Chargement en cours.", */
	/* SYS19 "Voguons vers", */
	/* SYS20 "Rester � bord", */
	/* SYS21 "Cit� de Cnossos", */
	/* SYS22 "Vall�e de l\'Indus", */
	/* SYS23 "Rapa Nui", */
	/* SYS24 "Mauvais fichier!", */
	/* SYS25 "ATTENTION|Vous allez charger|Une partie!", */
	/* SYS26 "ATTENTION|Vous allez sauvegarder|Une par-\ntie!", */
	/* SYS27 "Oui|Non", */
	/* SYS28 "Je ne vois rien � prendre ici!", */
	/* SYS29 "Je fouille, mais rien.", */
	/* SYS30 "Je ne trouve rien.", */
	/* SYS31 "FRANCAIS", */
	/* SYS32 "ENGLISH", */
/* }; */

/* Inventory Tooltip Index */
/* char *tooltip_dialogs[TOOLTIP_MAX_DIALOG] =
{ /*
	/* 00 "Crocus", */
	/* 01 "Amphore", */
	/* 02 "Caillou", */
	/* 03 "Tentacule", */
	/* 04 "Poisson frais", */
	/* 05 "Poisson saumure", */
	/* 06 "Brin de safran", */
	/* 07 "Gros ver", */
	/* 08 "Amphore pleine", */
	/* 09 "Poisson s�ch�", */
	/* 10 "Feuillage", */
	/* 11 "Marteau", */
	/* 12 "Cobra royal", */
	/* 13 "Eau", */
	/* 14 "Terre brute", */
	/* 15 "Boule d'argile", */
	/* 16 "Bousier affam�", */
	/* 17 "Bousier de compet", */
	/* 18 "Brindilles", */
	/* 19 "Bouse", */
	/* 20 "Hache", */
	/* 21 "Torche", */
	/* 22 "Medaillon 3", */
	/* 23 "Medaillon 2", */
	/* 24 "Statuette compl�te", */
	/* 25 "Masque mal�fique", */
	/* 26 "Statuette cass�e", */
	/* 27 "Obsidienne", */
/* }; */

/* Credits */
/* char *credits_dialogs[CREDITS_MAX_DIALOG] =
{ /*
	/* 00 "...", */
	/* 01 " ", */
	/* 02 "STOP", */
	/* 03 " ", */
	/* 04 "", */
	/* 05 " ", */
	/* 06 "----------------------------", */
	/* 07 " ", */
	/* 08 "A few  words  from the team", */
	/* 09 "behind "WINDLESS BAY  AMIGA"", */
	/* 10 " ", */
	/* 11 "", */
	/* 12 " ", */
	/* 13 "For   the  AMIGA  version,", */
	/* 14 "special  efforts and  tools", */
	/* 15 "were used  to get  the most", */
	/* 16 "of our beloved machine.    ", */
	/* 17 " ", */
	/* 18 "CODE:", */
	/* 19 "WINDLESS BAY  was ported to", */
	/* 20 "the Amiga  during  a course", */
	/* 21 "of ~3 years,  using VSCODE,", */
	/* 22 "GIT, PYTHON, WINUAE, SAS/C,", */
	/* 23 "a  MIST  (FPGA  Amiga)  and", */
	/* 24 "an ESCOM A1200 (thx Petro!)", */
	/* 25 "The last rush was completed", */
	/* 26 "on a MacBook M1 (ARM FTW!),", */
	/* 27 "using both FS-UAE & vAmiga.", */
	/* 28 " ", */
	/* 29 "99% of this game relies on", */
	/* 30 "the GRAPHICS LIBRARY.  This", */
	/* 31 "means that  the  whole game", */
	/* 32 "is meant to be os-friendly.", */
	/* 33 " ", */
	/* 34 "The game runs entirely in C", */
	/* 35 "except  for a couple of 3rd", */
	/* 36 "parties  unpacking routines", */
	/* 37 "(Shrinkler / NRV2x).       ", */
	/* 38 " ", */
	/* 39 "GRAPHICS:", */
	/* 40 "PHOTOSHOP   for   the  main", */
	/* 41 "part,  using either a WACOM", */
	/* 42 "BAMBOO  and  INTUOS tablets.", */
	/* 43 "Then COSMIGO PRO-MOTION and", */
	/* 44 "DELUXE PAINT,  for all  the", */
	/* 45 "pixel  and  dither tweaking.", */
	/* 46 " ", */
	/* 47 "MUSICS:", */
	/* 48 "Finally,  for those  who'd", */
	/* 49 "like  some  music technical", */
	/* 50 "details,  here's  what  was", */
	/* 51 "used  by David to make  the", */
	/* 52 "soundtracks:               ", */
	/* 53 "The PROTRACKER 3.15 as main", */
	/* 54 "sequencer.  Most of samples", */
	/* 55 "created  using  the  ROLAND", */
	/* 56 "JV-1080 &  the YAMAHA MOTIF", */
	/* 57 "XF.  Composition & research", */
	/* 58 "done on  the KORG TR-76 and", */
	/* 59 "YAMAHA PSR S500 keyboards. ", */
	/* 60 " ", */
	/* 61 "", */
	/* 62 " ", */
	/* 63 "And now, the  greetings for", */
	/* 64 "the  Amiga  version.       ", */
	/* 65 " ", */
	/* 66 "Many  thanks to  all these", */
	/* 67 "inspiring people:          ", */
	/* 68 " ", */
	/* 69 "4play", */
	/* 70 "Akaobi", */
	/* 71 "Aseyn", */
	/* 72 "BeeMixsy", */
	/* 73 "David  davidb2111_ Barbion", */
	/* 74 "Nicolas Bauw", */
	/* 75 "Batteman", */
	/* 76 "BjornNah", */
	/* 77 "C418", */
	/* 78 "Chris Covell", */
	/* 79 "Exocet", */
	/* 80 "FibreTigre", */
	/* 81 "Frost242", */
	/* 82 "Flopine", */
	/* 83 "Gelmir", */
	/* 84 "Gligli", */
	/* 85 "Jylam", */
	/* 86 "Kara N. Blohm", */
	/* 87 "MadMarie", */
	/* 88 "Mike 'DBug' Pointier", */
	/* 89 "MO5.COM", */
	/* 90 "Mooz", */
	/* 91 "Ninomojo", */
	/* 92 "NoRecess", */
	/* 93 "Oriens", */
	/* 94 "The Ptoing!", */
	/* 95 "Rahow", */
	/* 96 "RESISTANCE.NO", */
	/* 97 "Andr� & Louis-Marie Rocques", */
	/* 98 "Roudoudou", */
	/* 99 "Serge Fiedos", */
	/* 100 "St�phane F.", */
	/* 101 "Stingray", */
	/* 102 "Superchlo", */
	/* 103 "XBarr", */
	/* 104 "Yosshin4004", */
	/* 105 "Z-Team", */
	/* 106 "Zerkman", */
	/* 107 " ", */
	/* 108 "", */
	/* 109 " ", */
	/* 110 "External libraries:", */
	/* 111 "MT_ADPCM (MastaTabs)", */
	/* 112 "EasySound (Anders Bjerin)", */
	/* 113 "Miniz (Richard Geldreich)", */
	/* 114 "NRV2 (Ross)", */
	/* 115 "PTReplay.library (Stingray)", */
	/* 116 "Shrinkler (Blueberry)", */
	/* 117 " ", */
	/* 118 "", */
	/* 119 " ", */
	/* 120 "", */
	/* 121 " ", */
	/* 122 "", */
	/* 123 " ", */
	/* 124 "", */
	/* 125 " ", */
	/* 126 "", */
	/* 127 "WINDLESS BAY OUT!", */
	/* 128 "Press the mouse button", */
	/* 129 "to exit.", */
/* }; */

/* Dynamic dialogs indexes */
short dynamic_dialogs[GAME_MAX_DYNAMIC_DIALOG] = {2};
