/*
 * Find the platform-specific header for this platform.
 */

#ifndef PUTTY_PUTTYPS_H
#define PUTTY_PUTTYPS_H

#ifdef _WINDOWS

#include "winstuff.h"

#elif defined(macintosh)

#include "macstuff.h"

#elif defined(MACOSX)

#include "osx.h"

#elif defined(__BADA__)

#include "bada.h"

#else

#include "unix.h"

#endif

#endif
