#include <ctype.h>
#include <stdlib.h>

#include "putty.h"

FontSpec platform_default_fontspec(const char *name)
{
    FontSpec ret;
    ret.charset = 0;
    ret.height = 20;
    ret.isbold = 0;
    ret.name[0] = 0;
    return ret;
}

Filename platform_default_filename(const char *name)
{
    Filename ret;
    if (!strcmp(name, "LogFileName"))
		strcpy(ret.path, "/Home/.putty/putty.log");
    if (!strcmp(name, "PublicKeyFile"))
		strcpy(ret.path, "/Media/Others/putty.ppk");
    else
		*ret.path = '\0';
    return ret;
}

char *platform_default_s(const char *name)
{
    if (!strcmp(name, "SerialLine"))
		return dupstr("COM1");
    return NULL;
}

int platform_default_i(const char *name, int def)
{
    return def;
}
