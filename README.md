# build.h
Simple single header build system inspired by nobuild functionality

```c
#define BUILD_IMPLEMENTATION
#include "build.h"

int main(int argc, char *argv[])
{
        char *const result = strrchr(argv[0], '/');
        if (NULL != result)
                *result = '\0';
        char const *const src_dir = argv[0];

#ifdef _WIN32
        #define CC "cl"
#else
        #define CC "cc"
#endif

#define TARGET_BUILD "./build"
        #define TARGET_SRCS "./build.c"
                if (build_refresh("./build", (struct build_exe) {
                        .src_dir  = src_dir,
                        .deps     = BUILD_LIST(TARGET_SRCS),
                        .compiler = CC,
                        .srcs     = BUILD_LIST(TARGET_SRCS),
                })) return EXIT_SUCCESS;
        #undef TARGET_SRCS
#undef

#ifdef _WIN32
        #define TARGET_HELLO "./lib/hello.obj"
#else
        #define TARGET_HELLO "./lib/hello.o"
#endif
        #define TARGET_SRCS "./src/hello.c"
                build_lib(TARGET_HELLO, (struct build_lib) {
                        .src_dir  = src_dir,
                        .deps     = BUILD_LIST(TARGET_BUILD, TARGET_SRCS),
                        .inc_dirs = BUILD_LIST("./include"),
                        .compiler = CC,
                        .srcs     = BUILD_LIST(TARGET_SRCS),
                });
        #undef TARGET_SRCS
#undef

#define TARGET_MAIN "./bin/main"
        #define TARGET_SRCS "./lib/hello.o", "./src/main.c"
                build_exe(TARGET_MAIN, (struct build_exe) {
                        .src_dir  = src_dir,
                        .deps     = BUILD_LIST(TARGET_SRCS),
                        .compiler = CC,
                        .inc_dirs = BUILD_LIST("./include"),
                        .srcs     = BUILD_LIST(TARGET_SRCS),
                });
        #undef TARGET_SRCS
#undef
}
```
