#pragma once


#ifndef FORCE_INLINE
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		#define FORCE_INLINE __forceinline
	#else
		#define FORCE_INLINE inline
	#endif
#endif


#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define BUFFER_OFFSET(offset) ((char*)nullptr + offset)
#define UNUSED(x) (void)x


#define SAFE_DELETE(x) \
	if (x) \
	{ \
		delete x; \
		x = nullptr; \
	}
#define SAFE_DELETE_ARRAY(x) \
	if (x) \
	{ \
		delete[] x; \
		x = nullptr; \
	}
#define SAFE_RELEASE(x) \
	if (x) \
	{ \
		x->Release(); \
		x = nullptr; \
	}
