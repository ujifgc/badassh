#ifndef _BADA_H_
#define _BADA_H_

#include "bada_cross.h"

struct Filename {
    char path[1024];
};

typedef void *Context;

struct FontSpec {
    char name[64];
    int isbold;
    int height;
    int charset;
};

#define TICKSPERSEC 1000
#define sk_getxdmdata(socket, lenp) (NULL)

#define f_open(filename, mode, isprivate) ( fopen((filename).path, (mode)) )

#undef MSCRYPTOAPI

#define SEL_NL { 10 }

#define DEFAULT_CODEPAGE (0)

#define CURSORBLINK     (450)

void init_palette(void);
void cfgtopalette(void);

#endif
