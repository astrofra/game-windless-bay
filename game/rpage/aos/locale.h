#include "rpage/aos/inc.prl"
#include "rpage/frwk.h"

enum locale {locale_fr, locale_en, locale_es, locale_de};
extern char *locale_ext[3];

UBYTE *load_pak_locale_to_array(char *text_array[], UWORD array_size, UBYTE *packed_block, char *filename);
