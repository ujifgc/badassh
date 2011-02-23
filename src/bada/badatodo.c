#include <ctype.h>
#include <stdlib.h>

#include "putty.h"

// scrolling is totally unimplemented
void set_sbar(void *frontend, int total, int start, int page) {
}

// does any modern host beep?
void do_beep(void *frontend, int mode) {
}

// something mysterious
void update_specials_menu(void *frontend) {
}

/* TODO
 * Clipboard
 * must implement!
 */
void get_clip(void *frontend, wchar_t ** p, int *len) {
	*p = 0;
}

void write_clip(void *frontend, wchar_t * data, int *attr, int len, int must_deselect) {
}

/* TODO
 * Enthropy generation
 * must implement!
 */
void noise_get_light(void(*func)(void *, int)) {
}

void noise_get_heavy(void(*func)(void *, int)) {
}

void noise_regular(void) {
}

void noise_ultralight(unsigned long data) {
}

// mouse is unimplemented
void set_raw_mouse_mode(void *frontend, int activate) {
}

// at the time we ignore any resize requests from the host
void request_resize(void *frontend, int w, int h) {
}

// form icon change is not implemented
void set_icon(void *frontend, char *title) {
}

// form title change is not implemented
void set_title(void *frontend, char *title) {
}

// form iconization is not implemented
void set_iconic(void *frontend, int iconic) {
}

// form zooming is not implemented
void set_zoomed(void *frontend, int zoomed) {
}

// form is always maximized, no moving supported
void move_window(void *frontend, int x, int y) {
}

// form is always topmost, we can not move it
void set_zorder(void *frontend, int top) {
}

// unimplemented
void refresh_window(void *frontend) {
}

// form is never iconic
int is_iconic(void *frontend) {
	return 0;
}

// host never asked me that
void get_window_pos(void *frontend, int *x, int *y) {
	*x = 0;
	*y = 0;
}

// host never asked me that
void get_window_pixels(void *frontend, int *x, int *y) {
	*x = 800;
	*y = 800;
}

// host never asked me that
char *get_window_title(void *frontend, int icon) {
	return "";
}

// host never asked me that
void palette_reset(void *frontend) {
}

// host never asked me that
void palette_set(void *frontend, int n, int r, int g, int b) {
}
