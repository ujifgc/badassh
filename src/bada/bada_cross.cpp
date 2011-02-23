extern "C" {
#include "bada_cross.h"
}

#include <string.h>
#include <FIoFile.h>
#include <FIoDirectory.h>
#include <FSysSystemTime.h>

using namespace Osp::Io;
using namespace Osp::System;

extern "C" int mkdir(const char *filename) {
	result r = Directory::Create(filename, true);
	return E_SUCCESS != r;
}

extern "C" int rmdir(const char *filename) {
	result r = Directory::Remove(filename, true);
	return E_SUCCESS != r;
}

bool bada_IsDirExist(const char *dirname) {
	FileAttributes fa;
	result r = File::GetAttributes(dirname, fa);
	if (E_SUCCESS != r)
		return false;
	return fa.IsDirectory();
}

extern "C" void * bada_memmove(void * dst, const void * src, int count) {
	void * ret = dst;

	if (dst <= src || (char *) dst >= ((char *) src + count)) {
		/*
		 * Non-Overlapping Buffers
		 * copy from lower addresses to higher addresses
		 */
		while (count--) {
			*(char *) dst = *(char *) src;
			dst = (char *) dst + 1;
			src = (char *) src + 1;
		}
	} else {
		/*
		 * Overlapping Buffers
		 * copy from higher addresses to lower addresses
		 */
		dst = (char *) dst + count - 1;
		src = (char *) src + count - 1;

		while (count--) {
			*(char *) dst = *(char *) src;
			dst = (char *) dst - 1;
			src = (char *) src - 1;
		}
	}

	return (ret);
}

extern "C" void * bada_memcpy (void * dst, const void * src, int count) {
	void * ret = dst;

	/*
	 * copy from lower addresses to higher addresses
	 */
	while (count--) {
		*(char *)dst = *(char *)src;
		dst = (char *)dst + 1;
		src = (char *)src + 1;
	}

	return(ret);
}

extern "C" int access(const char *path, int amode) {
	switch (amode) {
	case 0:
		return File::IsFileExist(path) ? 0 : -1;
	default:
		return -1;
	}
}

int ftruncate(int, int) {
	return 0;
}

extern "C" int execlp(const char *file, ...) {
	return 0;
}

extern "C" int execvp(const char *file, ...) {
	return 0;
}

extern "C" int GETTICKCOUNT() {
	long long ticks;
	SystemTime::GetTicks(ticks);
	return (int) ticks;
}

extern "C" int bada_Rename(const char *src, const char *dst) {
	result r = File::Remove(dst);
	if (E_SUCCESS == r || E_FILE_NOT_FOUND == r) {
		if (E_SUCCESS == File::Move(src, dst))
			return E_SUCCESS;
		if (E_SUCCESS == File::Copy(src, dst, false))
			return E_SUCCESS;
	}
	return E_FAILURE;
}
