# build.h
Simple single header build system inspired by nobuild functionality

```c
#define BUILD_IMPLEMENTATION
#include "build.h"

#ifdef _WIN32
        #define CC "cl"
#else
        #define CC "cc"
#endif

int main(int argc, char *argv[])
{
        char *const result = strrchr(argv[0], '/');
        char const *src_dir = ".";
        if (NULL != result) {
                *result = '\0';
                src_dir = argv[0];
        }

        char const *const target_build =
                "./build" BUILD_EXE;
        if (build_refresh(target_build, (struct build_exe) {
                .src_dir  = src_dir,
                .deps     = BUILD_LIST(__FILE__),
                .compiler = CC,
                .srcs     = BUILD_LIST(__FILE__),
        })) return EXIT_SUCCESS;

        char const *const target_hello =
                "./lib/hello" BUILD_SHARED;
        build_lib(target_hello, (struct build_lib) {
                .src_dir  = src_dir,
                .deps     = BUILD_LIST(target_build, "./src/hello.c"),
                .inc_dirs = BUILD_LIST("./include"),
                .compiler = CC,
                .srcs     = BUILD_LIST("./src/hello.c"),
        });

        char const *const target_main =
                "./bin/main" BUILD_EXE;
        build_exe(target_main, (struct build_exe) {
                .src_dir  = src_dir,
                .deps     = BUILD_LIST(target_hello, "./src/main.c"),
                .compiler = CC,
                .inc_dirs = BUILD_LIST("./include"),
                .srcs     = BUILD_LIST("./src/main.c"),
                .libs     = BUILD_LIST(target_hello),
        });
}
```
