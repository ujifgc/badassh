#include <ctype.h>
#include <stdlib.h>

#include "putty.h"
#include "terminal.h"
#include "bada_cross.h"
#include "virtkeys.h"

Filename filename_from_str(const char *str) {
	Filename ret;
	strncpy(ret.path, str, sizeof(ret.path));
	ret.path[sizeof(ret.path) - 1] = '\0';
	return ret;
}

const char *filename_to_str(const Filename *fn) {
	return fn->path;
}

int filename_is_null(Filename fn) {
	return !*fn.path;
}

int filename_equal(Filename f1, Filename f2)
{
    return !strcmp(f1.path, f2.path);
}
