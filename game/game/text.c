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
	/* 0 "Le pupitre de la station d\'écoute.\nD\'ici crépitent les ondes du monde en-\ntier.", */
	/* 1 "Vous êtes dans le local technique.\nOn y trouve un peu de tout.", */
	/* 2 "Le code de dévérouillage est le %s.", */
	/* 3 "Au pied de la tour radio, le froid à\nl\'exterieur est difficilement suppor-\ntable.", */
	/* 4 "La lande à perte de vue.\nDerrière les montagnes...", */
	/* 5 "Vous êtes au bord du monde, un puit natu-\nrel crache de la vapeur sans disconti-\nnuer.", */
	/* 6 "Une autre station radio? Ou bien est-ce\nune habitation?", */
	/* 7 "A l\'interieur tout est vide.\nUne porte semble mener au sous-sol.", */
	/* 8 "Le sous-sol était en réalité un gigan-\ntesque puit d\'aeration.", */
	/* 9 "Un long tunnel porte quelques traces de.", */
	/* 10 "Une lourde porte semble mener vers l\'ex-\nterieur.\npassage relativement récentes.", */
	/* 11 "Une splendide antenne radar vous observe\ndans un silence absolu.", */
	/* 12 "Vous longer un canal vide et désolé, une\nrumeur maritime vous parvient depuis le\nsud.", */
	/* 13 "Quelqu\'un était ici il y a peu. Un feu\nimprovisé vous rechauffe les mains.", */
	/* 14 "Le port, désert, mais les installations\nsont encore en place !", */
	/* 15 "Sur les quais, les entrées maritimes vous\nappellent.", */
	/* 16 "En haute mer, a des lieues de la côté,\nvous accostez sur une station maritime!", */
	/* 17 "Le hangar à bateaux, dans un style digne\ndes constructions Eiffel!", */
	/* 18 "Un petit salon, le cadre est accueillant.", */
/* };
*/

/* System dialogs */
/* char *system_dialogs[SYS_MAX_DIALOG] =
{ */
	/* SYS00 "", */
	/* SYS01 "C\'est dans la poche.", */
	/* SYS02 "Je ne vois rien d\'intéressant ici!", */
	/* SYS03 "J\'ai beau scruter, il n\'y a rien.", */
	/* SYS04 "Rien de notable.", */
	/* SYS05 "Ne perdez pas de temps.", */
	/* SYS06 "Laissez-moi tranquille, je n\'ai rien à\nvous dire.", */
	/* SYS07 "Je vous ai tout dit.", */
	/* SYS08 "Je ne peux pas faire ça.", */
	/* SYS09 "Vous n\'utilisez pas le bon objet!", */
	/* SYS10 "ça ne donne rien.", */
	/* SYS11 "Vous avez déjà ce qu\'il vous faut!", */
	/* SYS12 "Charger partie", */
	/* SYS13 "Sauver partie", */
	/* SYS14 "Partie sauvegardée!", */
	/* SYS15 "Insérez disquette", */
	/* SYS16 "Insérez disquette 2", */
	/* SYS17 "Appuyez sur le bouton de la souris", */
	/* SYS18 "Chargement d\'Athanor 2", */
	/* SYS19 "Voguons vers", */
	/* SYS20 "Rester à bord", */
	/* SYS21 "Cité de Cnossos", */
	/* SYS22 "Vallée de l\'Indus", */
	/* SYS23 "Rapa Nui", */
	/* SYS24 "Mauvais fichier!", */
	/* SYS25 "ATTENTION|Vous allez charger|Une partie!", */
	/* SYS26 "ATTENTION|Vous allez sauvegarder|Une par-\ntie!", */
	/* SYS27 "Oui|Non", */
	/* SYS28 "Je ne vois rien à prendre ici!", */
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
	/* 09 "Poisson séché", */
	/* 10 "Feuillage", */
	/* 11 "Marteau", */
	/* 12 "Cobra royal", */
	/* 13 "Eau", */
	/* 14 "Terre brute", */
	/* 15 "Boule d'argile", */
	/* 16 "Bousier affamé", */
	/* 17 "Bousier de compet", */
	/* 18 "Brindilles", */
	/* 19 "Bouse", */
	/* 20 "Hache", */
	/* 21 "Torche", */
	/* 22 "Medaillon 3", */
	/* 23 "Medaillon 2", */
	/* 24 "Statuette complète", */
	/* 25 "Masque maléfique", */
	/* 26 "Statuette cassée", */
	/* 27 "Obsidienne", */
/* }; */

/* Credits */
/* char *credits_dialogs[CREDITS_MAX_DIALOG] =
{ /*
	/* 00 " ", */
	/* 01 "Crée et Réalisé par", */
	/* 02 "ERIC SAFAR", */
	/* 03 " ", */
	/* 04 "Moteur & Atari code", */
	/* 05 "ERIC SAFAR", */
	/* 06 " ", */
	/* 07 "Amiga code", */
	/* 08 "FRANCOIS GUTHERZ", */
	/* 09 " ", */
	/* 10 "Illustrations", */
	/* 11 "ANGEL BAUTISTA", */
	/* 12 " ", */
	/* 13 "Infographie", */
	/* 14 "FRANCOIS GUTHERZ", */
	/* 15 "VINCENT JAMBUT", */
	/* 16 " ", */
	/* 17 "Atari Musics", */
	/* 18 "MATHIEU STEMPELL", */
	/* 19 " ", */
	/* 20 "Amiga Musics", */
	/* 21 "DAVID VANDENSTEEN", */
	/* 22 " ", */
	/* 23 "Traduction Anglaise", */
	/* 24 "ERNEST SAFAR", */
	/* 25 "CHEN CHEN", */
	/* 26 " ", */
	/* 27 "Conception manuel", */
	/* 28 "JEAN-FRANCOIS REVEL", */
	/* 29 " ", */
	/* 30 "Routines additionnelles", */
	/* 31 "GT-Turbo", */
	/* 32 "SPEED PACKER V3", */
	/* 33 "maxYMiser Replayer", */
	/* 34 "WIZZCAT ROUTINE", */
	/* 35 "THE SPRITEWORKS KIT", */
	/* 36 " ", */
	/* 37 "Remerciements Spéciaux", */
	/* 38 "REMI HERBULOT", */
	/* 39 "DANIELE HERBULOT", */
	/* 40 " ", */
	/* 41 "Remerciements", */
	/* 42 "FRANK OSTROWSKI", */
	/* 43 "MON AMI ROGER BOUR", */
	/* 44 "MISS POLLY", */
	/* 45 "ATARI LEGEND", */
	/* 46 "RGC & AC CONVENTIONS TEAM", */
	/* 47 "PRINCEPS-BONUS", */
	/* 48 "Freesound.org", */
	/* 49 "Dr Floyd from GAMOPAT", */
	/* 50 "All the SILICIUM members", */
	/* 51 "All the GFA forum members", */
	/* 52 "All the 3614 RTEL members", */
	/* 53 "PHF Intro", */
	/* 54 "LAURENT VICOMTE", */
	/* 55 "TCB", */
	/* 56 "THE REPLICANTS", */
	/* 57 "Steve Bak (RIP)", */
	/* 58 "Jean Martial Lefranc", */
	/* 59 "Philippe Ulrich", */
	/* 60 "Patrick Dublanchet", */
	/* 61 "Olivier Robin", */
	/* 62 "Christophe Le Bouil", */
	/* 63 "Eric Gachons", */
	/* 64 "Ugo Robain", */
	/* 65 "Manuel Alvarez", */
	/* 66 "Nicolas Gohin (RIP l'ami)", */
	/* 67 "Olivier Morazé", */
	/* 68 "Didier Quentin", */
	/* 69 "Franck Quero", */
	/* 70 " ", */
	/* 71 "Salutations de Mathieu Stempell", */
	/* 72 "ST Survivor / NLC", */
	/* 73 "gwEm / PHF", */
	/* 74 "Cooper / Paradise", */
	/* 75 "Frost / Sector One", */
	/* 76 "Zerkman / Sector One", */
	/* 77 "Mic / Dune", */
	/* 78 "Chuck / Dune", */
	/* 79 "Thadoss / Dune", */
	/* 80 "Lotek Style / TSCC", */
	/* 81 "Tomchi / MJJ Prod", */
	/* 82 "Havoc / Lineout", */
	/* 83 "Evil / DHS", */
	/* 84 "bob_er / MEC", */
	/* 85 "Cyg / BlaBla", */
	/* 86 "Bod / Stax (RIP)", */
	/* 87 "Other Atari demosceners", */
	/* 88 "Hally / VORC", */
	/* 89 "Talus / Kohina", */
	/* 90 "Stefan Lindberg", */
	/* 91 "All at atari-forum", */
	/* 92 "StickHead", */
	/* 93 "Cogweasel", */
	/* 94 "BouleDeFeu crew", */
	/* 95 " ", */
	/* 96 "Eric's Playlist", */
	/* 97 "Barclay James Harvest", */
	/* 98 "Ten years after", */
	/* 99 "Dire Straits", */
	/* 100 "AC/DC", */
	/* 101 "The Strokes", */
	/* 102 "Blue Oster Cult", */
	/* 103 "Curtis Harding", */
	/* 104 "Dinah Washington", */
	/* 105 "J.S Bach", */
	/* 106 "W.A Mozart", */
	/* 107 " ", */
	/* 108 "STOP", */
	/* 109 " ", */
	/* 110 "", */
	/* 111 " ", */
	/* 112 "----------------------------", */
	/* 113 " ", */
	/* 114 "Quelques  mots  de  la team", */
	/* 115 ""ATHANOR  2,  AMIGA VERSION"", */
	/* 116 " ", */
	/* 117 "", */
	/* 118 " ", */
	/* 119 "Voici  avec  quels   outils", */
	/* 120 "nous  avons  travaillé afin", */
	/* 121 "d'exploiter au mieux  votre", */
	/* 122 "machine favorite:          ", */
	/* 123 " ", */
	/* 124 "CODE :", */
	/* 125 "Le   travail   d'adaptation", */
	/* 126 "d'Athanor  2  vers  l'Amiga", */
	/* 127 "a pris ~3 ans,  a l'aide de", */
	/* 128 "VSCODE,  UAE,  PYTHON, GIT,", */
	/* 129 "SAS/C, un MIST (Amiga FPGA)", */
	/* 130 "& un A1200 ESCOM (Petro <3)", */
	/* 131 "La fin  du  développement a", */
	/* 132 "été faite sur un MacBook M1", */
	/* 133 "avec FS-UAE et vAmiga.     ", */
	/* 134 " ", */
	/* 135 "99% d'Athanor est basé sur", */
	/* 136 "la GRAPHICS.LIBRARY! Ce qui", */
	/* 137 "veut  dire que la  totalité", */
	/* 138 "du code  est "OS-friendly".", */
	/* 139 " ", */
	/* 140 "Le jeu est  tout écrit en C", */
	/* 141 "sauf  quelques  routines de", */
	/* 142 "décompression  des  données", */
	/* 143 "(Shrinkler / NRV2x).       ", */
	/* 144 " ", */
	/* 145 "GRAPHISMES :", */
	/* 146 "PHOTOSHOP,  principalement,", */
	/* 147 "sur  WACOM BAMBOO / INTUOS.", */
	/* 148 "Puis,   COSMIGO  PRO-MOTION", */
	/* 149 "et DELUXE PAINT,  pour  les", */
	/* 150 "finition & trames au pixel.", */
	/* 151 " ", */
	/* 152 "MUSIQUE :", */
	/* 153 "Pour  ceux  qui  aiment les", */
	/* 154 "détails  techniques,  voici", */
	/* 155 "les  outils  utilisés   par", */
	/* 156 "David  pour  la bande son :", */
	/* 157 "PROTRACKER   v3.15,   comme", */
	/* 158 "séquenceur. La plupart  des", */
	/* 159 "samples     viennent    des", */
	/* 160 "ROLAND  JV-1080  et  YAMAHA", */
	/* 161 "MOTIF  XF.  La  composition", */
	/* 162 "et  les recherches ont  été", */
	/* 163 "faites   sur  des  claviers", */
	/* 164 "KORG  TR-76 et  YAMAHA  PSR", */
	/* 165 "S500.                      ", */
	/* 166 " ", */
	/* 167 "", */
	/* 168 " ", */
	/* 169 "Nos  remerciements  pour la", */
	/* 170 "version Amiga.             ", */
	/* 171 "Merci à vous  tous  pour la", */
	/* 172 "motivation & l'inspiration:", */
	/* 173 " ", */
	/* 174 "4play", */
	/* 175 "Akaobi", */
	/* 176 "Aseyn", */
	/* 177 "BeeMixsy", */
	/* 178 "David  davidb2111_ Barbion", */
	/* 179 "Nicolas Bauw", */
	/* 180 "Batteman", */
	/* 181 "BjornNah", */
	/* 182 "C418", */
	/* 183 "Chris Covell", */
	/* 184 "Exocet", */
	/* 185 "FibreTigre", */
	/* 186 "Frost242", */
	/* 187 "Flopine", */
	/* 188 "Gelmir", */
	/* 189 "Gligli", */
	/* 190 "Jylam", */
	/* 191 "Kara N. Blohm", */
	/* 192 "MadMarie", */
	/* 193 "Mike 'DBug' Pointier", */
	/* 194 "MO5.COM", */
	/* 195 "Mooz", */
	/* 196 "Ninomojo", */
	/* 197 "NoRecess", */
	/* 198 "Oriens", */
	/* 199 "The Ptoing!", */
	/* 200 "Rahow", */
	/* 201 "RESISTANCE.NO", */
	/* 202 "André & Louis-Marie Rocques", */
	/* 203 "Roudoudou", */
	/* 204 "Serge Fiedos", */
	/* 205 "Stéphane F.", */
	/* 206 "Stingray", */
	/* 207 "Superchlo", */
	/* 208 "XBarr", */
	/* 209 "Yosshin4004", */
	/* 210 "Z-Team", */
	/* 211 "Zerkman", */
	/* 212 " ", */
	/* 213 "", */
	/* 214 " ", */
	/* 215 "Libs. externes:", */
	/* 216 "MT_ADPCM (MastaTabs)", */
	/* 217 "EasySound (Anders Bjerin)", */
	/* 218 "Miniz (Richard Geldreich)", */
	/* 219 "NRV2 (Ross)", */
	/* 220 "PTReplay.library (Stingray)", */
	/* 221 "Shrinkler (Blueberry)", */
	/* 222 " ", */
	/* 223 "", */
	/* 224 " ", */
	/* 225 "", */
	/* 226 " ", */
	/* 227 "", */
	/* 228 " ", */
	/* 229 "", */
	/* 230 " ", */
	/* 231 "", */
	/* 232 "ATHANOR OUT!", */
	/* 233 "Appuyez sur le bouton de", */
	/* 234 "la souris pour sortir.", */
/* }; */

/* Dynamic dialogs indexes */
short dynamic_dialogs[GAME_MAX_DYNAMIC_DIALOG] = {2};
