#ifndef BADA_CROSS_H
#define BADA_CROSS_H

int printf (const char *__format, ...);

int mkdir(const char *);

int ftruncate(int, int);

int GETTICKCOUNT();

int sscanf(const char *text, const char *fmt, ...);
int sprintf(const char *text, const char *fmt, ...);

int bada_Rename(const char *text, const char *fmt);

void * bada_memmove (void * dst, const void * src, int count);
void * bada_memcpy (void * dst, const void * src, int count);

#endif
