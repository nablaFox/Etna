// TEMPORARY: waiting for GCC 15 #embed with
// #embed we will be able of defining static data
// and hence the linker will strip away unused
// shaders binaries

#define _EMBED_BINARY(name, file)            \
	asm(".section .rodata\n"                 \
		".global " #name                     \
		"\n"                                 \
		".type " #name ", @object\n" #name   \
		":\n"                                \
		".incbin \"" file "\"\n" #name       \
		"_end:\n"                            \
		".size " #name ", .-" #name          \
		"\n"                                 \
		".previous\n");                      \
	extern const unsigned char name[];       \
	extern const unsigned char name##_end[]; \
	const size_t name##_size = (size_t)(name##_end - name);

#define EMBED_BINARY(name, file) _EMBED_BINARY(name, ETNA_SOURCE_DIR "/" file)
