#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <limits.h>
#include <wchar.h>
#include <time.h>

extern "C" {
#include "putty.h"
#include "terminal.h"
#include "misc.h"
}

int is_dbcs_leadbyte(int codepage, char byte) {
	return 0; /* we don't do DBCS */
}

int mb_to_wc(int codepage, int flags, char *mbstr, int mblen, wchar_t *wcstr, int wclen) {
	return mbstowcs(wcstr, mbstr, wclen);
}

int wc_to_mb(int codepage, int flags, wchar_t *wcstr, int wclen, char *mbstr, int mblen, char *defchr, int *defused,
		struct unicode_data *ucsdata) {
	return wcstombs(mbstr, wcstr, mblen);
}

void init_ucs(Config *cfg, struct unicode_data *ucsdata) {
	ucsdata->line_codepage = CP_UTF8;
	for (int i = 0; i < 256; i++)
		if (ucsdata->unitab_line[i] < ' ' || (ucsdata->unitab_line[i] >= 0x7F && ucsdata->unitab_line[i] < 0xA0))
			ucsdata->unitab_ctrl[i] = i;
		else
			ucsdata->unitab_ctrl[i] = 0xFF;
}
