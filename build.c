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
        if (NULL != result)
                *result = '\0';
        char const *const src_dir = argv[0];
        
        if (build_refresh("./build", (struct build_exe) {
                .src_dir  = src_dir,
                .deps     = BUILD_LIST(__FILE__),
                .compiler = CC,
                .srcs     = BUILD_LIST(__FILE__),
        })) return EXIT_SUCCESS;
        
        build_lib("./lib/hello.o", (struct build_lib) {
                .src_dir  = src_dir,
                .deps     = BUILD_LIST("./src/hello.c", "./build"),
                .inc_dirs = BUILD_LIST("./include"),
                .compiler = CC,
                .srcs     = BUILD_LIST("./src/hello.c"),
        });
        
        build_exe("./bin/main" BUILD_EXTENSION, (struct build_exe) {
                .src_dir  = src_dir,
                .deps     = BUILD_LIST("./src/main.c", "./lib/hello.o"),
                .compiler = CC,
                .inc_dirs = BUILD_LIST("./include"),
                .srcs     = BUILD_LIST("./lib/hello.o", "./src/main.c"),
        });
}
