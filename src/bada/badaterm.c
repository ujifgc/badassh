#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#include "putty.h"
#include "terminal.h"

Terminal *term;

void frontend_keypress(void *handle) {
	return;
}

/* Dummy routine, only required in plink. */
void ldisc_update(void *frontend, int echo, int edit) {
}

char *get_ttymode(void *frontend, const char *mode) {
	return term_get_ttymode(term, mode);
}

int get_userpass_input(prompts_t *p, unsigned char *in, int inlen) {
	return term_get_userpass_input(term, p, in, inlen);
}

int from_backend(void *frontend, int is_stderr, const char *data, int len)
{
    return term_data(term, is_stderr, data, len);
}

int from_backend_untrusted(void *frontend, const char *data, int len)
{
    return term_data_untrusted(term, data, len);
}
