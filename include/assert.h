#define assert(expr) \
    if (expr){ \
    } else {\
        iprintf("Assertion failed:\n	%s\n	%s:%d", #expr, __FILE__, __LINE__);\
		while(1);\
	}
 
#define checkme assert(this != NULL && this->magic == MAGIC)

#define glTexCoord1i(x) GFX_TEX_COORD = x

