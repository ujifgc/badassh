#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

extern "C" {
#include "putty.h"
#include "terminal.h"
}
#include "Form1.h"

extern Config cfg;
extern Terminal *term;
extern Form1 *mainForm;

#include <FBase.h>
#include <FUi.h>
#include <FUiCtrlScrollPanel.h>
#include <FBaseColIList.h>

using namespace Osp::Base;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;
using namespace Osp::Graphics;
using namespace Osp::Base::Collection;

#define FONT_NORMAL 0
#define FONT_BOLD 1
#define FONT_UNDERLINE 2
#define FONT_BOLDUND 3
#define FONT_MAXNO 	4

#define NCFGCOLOURS 22
#define NEXTCOLOURS 240
#define NALLCOLOURS (NCFGCOLOURS + NEXTCOLOURS)
static Color colours[NALLCOLOURS];
static Color defpal[NALLCOLOURS];

extern int font_width, font_height, font_dualwidth;
extern int offset_width, offset_height;
extern unicode_data ucsdata;

static Font *fonts[FONT_MAXNO] = {0};
typedef struct {
	char name[20];
	int style;
	int size;
} FontType;
FontType lfont;
static int fontflag[FONT_MAXNO];
static enum {
	BOLD_COLOURS, BOLD_SHADOW, BOLD_FONT
} bold_mode;
static enum {
	UND_LINE, UND_FONT
} und_mode;

void init_fonts(int pick_width, int pick_height) {
	int i;

	for (i = 0; i < FONT_MAXNO; i++) {
		if (fonts[i]) delete fonts[i];
		fonts[i] = NULL;
	}

	bold_mode = cfg.bold_colour ? BOLD_COLOURS : BOLD_FONT;
	und_mode = UND_FONT;

	fonts[FONT_NORMAL] = new Font;
	fonts[FONT_NORMAL]->Construct("/Res/LiberationMono-Regular.ttf", FONT_STYLE_PLAIN, pick_height);
	Dimension dim;
	fonts[FONT_NORMAL]->GetTextExtent("1234567890ABCDEFGHIJ", 20, dim);
	font_width = dim.width / 20;
	font_height = dim.height;

	fonts[FONT_BOLD] = new Font;
	fonts[FONT_BOLD]->Construct("/Res/LiberationMono-Regular.ttf", FONT_STYLE_BOLD, pick_height);

	fonts[FONT_UNDERLINE] = new Font;
	fonts[FONT_UNDERLINE]->Construct("/Res/LiberationMono-Regular.ttf", FONT_STYLE_PLAIN, pick_height);
	fonts[FONT_UNDERLINE]->SetUnderline(true);

	fontflag[0] = fontflag[1] = fontflag[2] = 1;

	init_ucs(&cfg, &ucsdata);
}

/*
 * Draw a line of text in the window, at given character
 * coordinates, in given attributes.
 *
 * We are allowed to fiddle with the contents of `text'.
 */
void do_text_internal(Context ctx, int x, int y, wchar_t *text, int len, unsigned long attr, int lattr) {
	int nfg, nbg, nfont, t;
	Canvas *can = (Canvas *) ctx;
	int fnt_width, char_width;
	int text_adjust = 0;
	static int *IpDx = 0, IpDxLEN = 0;

	lattr &= LATTR_MODE;

	char_width = fnt_width = font_width * (1 + (lattr != LATTR_NORM));

	if (attr & ATTR_WIDE)
		char_width *= 2;

	if (len > IpDxLEN || IpDx[0] != char_width) {
		int i;
		if (len > IpDxLEN) {
			sfree(IpDx);
			IpDx = snewn(len + 16, int);
			IpDxLEN = (len + 16);
		}
		for (i = 0; i < IpDxLEN; i++)
			IpDx[i] = char_width;
	}

	/* Only want the left half of double width lines */
	if (lattr != LATTR_NORM && x * 2 >= term->cols)
		return;

	x *= fnt_width;
	y *= font_height;
	x += offset_width;
	y += offset_height;

	if ((attr & TATTR_ACTCURS) && (cfg.cursor_type == 0 || term->big_cursor)) {
		attr &= ~(ATTR_REVERSE | ATTR_BLINK | ATTR_COLOURS);
		if (bold_mode == BOLD_COLOURS)
			attr &= ~ATTR_BOLD;

		/* cursor fg and bg */
		attr |= (260 << ATTR_FGSHIFT) | (261 << ATTR_BGSHIFT);
	}

	nfont = 0;

	/* Anything left as an original character set is unprintable. */
	if (DIRECT_CHAR(text[0])) {
		int i;
		for (i = 0; i < len; i++)
			text[i] = 0xFFFD;
	}

	nfg = ((attr & ATTR_FGMASK) >> ATTR_FGSHIFT);
	nbg = ((attr & ATTR_BGMASK) >> ATTR_BGSHIFT);
	if (bold_mode == BOLD_FONT && (attr & ATTR_BOLD))
		nfont |= FONT_BOLD;
	if (und_mode == UND_FONT && (attr & ATTR_UNDER))
		nfont |= FONT_UNDERLINE;
	if (!fonts[nfont])
		nfont = FONT_NORMAL;
	if (attr & ATTR_REVERSE) {
		t = nfg;
		nfg = nbg;
		nbg = t;
	}
	if (bold_mode == BOLD_COLOURS && (attr & ATTR_BOLD)) {
		if (nfg < 16)
			nfg |= 8;
		else if (nfg >= 256)
			nfg |= 1;
	}
	if (bold_mode == BOLD_COLOURS && (attr & ATTR_BLINK)) {
		if (nbg < 16)
			nbg |= 8;
		else if (nbg >= 256)
			nbg |= 1;
	}

	can->SetFont(*fonts[nfont]);
	can->SetForegroundColor(colours[nfg]);
	can->SetBackgroundColor(colours[nbg]);

	can->FillRectangle(colours[nbg], Rectangle(x,y - font_height * (lattr == LATTR_BOT) + text_adjust, len*font_width, font_height));
	can->DrawText(Point(x, y - font_height * (lattr == LATTR_BOT) + text_adjust), String(text), len);
}

/*
 * Wrapper that handles combining characters.
 */
void do_text(Context ctx, int x, int y, wchar_t *text, int len, unsigned long attr, int lattr) {
	if (attr & TATTR_COMBINING) {
		unsigned long a = 0;
		attr &= ~TATTR_COMBINING;
		while (len--) {
			do_text_internal(ctx, x, y, text, 1, attr | a, lattr);
			text++;
			a = TATTR_COMBINING;
		}
	} else
		do_text_internal(ctx, x, y, text, len, attr, lattr);
}

/*
 * This function gets the actual width of a character in the normal font.
 */
int char_width(Context ctx, int uc) {
	return 1;
}

void do_cursor(Context ctx, int x, int y, wchar_t *text, int len, unsigned long attr, int lattr) {
	mainForm->ScrollCursor();
	return;
}

void init_palette(void) {
	for (int i = 0; i < NALLCOLOURS; i++)
		colours[i] = defpal[i];
}

void cfgtopalette(void) {
	int i;
	static const int ww[] = { 256, 257, 258, 259, 260, 261, 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15 };

	for (i = 0; i < 22; i++) {
		int w = ww[i];
		unsigned char r = cfg.colours[i][0], g = cfg.colours[i][1], b = cfg.colours[i][2];
		defpal[w] = Color(r, g, b);
	}
	for (i = 0; i < NEXTCOLOURS; i++) {
		if (i < 216) {
			int r = i / 36, g = (i / 6) % 6, b = i % 6;
			defpal[i + 16].SetRed(r ? r * 40 + 55 : 0);
			defpal[i + 16].SetGreen(g ? g * 40 + 55 : 0);
			defpal[i + 16].SetBlue(b ? b * 40 + 55 : 0);
		} else {
			int shade = i - 216;
			shade = shade * 10 + 8;
			defpal[i + 16] = Color(shade, shade, shade);
		}
	}
}

void request_paste(void *frontend)
{
    term_do_paste(term);
}
