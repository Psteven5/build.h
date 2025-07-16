#ifndef BUILD_H_
#define BUILD_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
        #include <windows.h>

        #define BUILD_EXTENSION ".exe"
#else
        #define BUILD_EXTENSION
#endif

#ifdef _MSC_VER
        #define BUILD_LIST(...) ((char const *[]) {            \
                __VA_ARGS__, NULL                              \
        })
#else
        #define BUILD_LIST(...) ((char const *[]) {            \
                __VA_ARGS__ BUILD_VA_OPT_(__VA_ARGS__)(,) NULL \
        })
#endif

#define BUILD_VA_OPT_(    ...) BUILD_IF_  (BUILD_IS_PROBE_  (\
                               BUILD_IF_  (BUILD_IS_PROBE_  (\
                               BUILD_HEAD_(__VA_ARGS__)))(~)(\
                               BUILD_HEAD_(__VA_ARGS__)) (~)))()
#define BUILD_CAT_(       ...) BUILD_CAT__(__VA_ARGS__)
#define BUILD_CAT__(X,    ...) X ##        __VA_ARGS__
#define BUILD_IF_(        ...) BUILD_IF__ (BUILD_NOT_(__VA_ARGS__))
#define BUILD_IF__(       ...) BUILD_CAT_ (BUILD_IF_, __VA_ARGS__)
#define BUILD_IF_0(       ...) __VA_ARGS__ BUILD_ELS1_
#define BUILD_IF_1(       ...)             BUILD_ELS0_
#define BUILD_ELS0_(      ...) __VA_ARGS__
#define BUILD_ELS1_(      ...)
#define BUILD_NOT_(       ...) BUILD_IS_PROBE_ (\
                               BUILD_CAT_      (BUILD_NOT_,  __VA_ARGS__))
#define BUILD_IS_PROBE_(  ...) BUILD_IS_PROBE__(BUILD_PROBE_ __VA_ARGS__)
#define BUILD_IS_PROBE__( ...) BUILD_SND_      (__VA_ARGS__, 0,)
#define BUILD_PROBE_(     ...) ~, 1
#define BUILD_SND_( _, X, ...) X
#define BUILD_NOT_0            BUILD_PROBE_(~)
#define BUILD_HEAD_(X,    ...) X

struct build_lib {
        char const        *src_dir;
        char const *const *deps;
        char const        *compiler;
        char const *const *cflags,
                   *const *inc_dirs,
                   *const *flags,
                   *const *srcs,
                   *const *lib_dirs,
                   *const *libs;
        int                is_static,
                           is_msvc,
                           is_silent;
};

struct build_exe {
        char const        *src_dir;
        char const *const *deps;
        char const        *compiler;
        char const *const *cflags,
                   *const *inc_dirs,
                   *const *flags,
                   *const *srcs,
                   *const *lib_dirs,
                   *const *libs;
        int                is_static,
                           is_msvc,
                           is_silent;
};

extern struct build_buffer_ {
        char    *data;
        unsigned count,
                 capacity;
} build_buffer_;

static inline void build_buffer_reserve_(unsigned new_capacity)
{
        if (new_capacity < build_buffer_.capacity)
                return;
        for (unsigned i = 1; i != 8 * sizeof(i); i <<= 1)
                new_capacity |= new_capacity >> i;
        build_buffer_.capacity = 1 + new_capacity;
        build_buffer_.data = realloc(build_buffer_.data, build_buffer_.capacity);
}

static inline void build_buffer_appendf_(char const *restrict fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        unsigned const count =
        #ifdef _WIN32
                _vscprintf(fmt, args);
        #else
                vsnprintf(NULL, 0, fmt, args);
        #endif
        va_end(args);
        build_buffer_reserve_(count + build_buffer_.count);
        va_start(args, fmt);
        vsnprintf(build_buffer_.data + build_buffer_.count,
                build_buffer_.capacity - build_buffer_.count, fmt, args);
        va_end(args);
        build_buffer_.count += count;
}

static inline void build_full_path_ (
        char const *restrict target,
        char const *restrict src_dir)
{
#ifdef _WIN32
        unsigned i = build_buffer_.count;
#endif
        if (NULL == src_dir)
                src_dir = ".";
        if (target == strstr(target, "./")
#ifdef _WIN32
         || target == strstr(target, ".\\")
        )
                build_buffer_appendf_("%s\\%s", src_dir, 2 + target);
#else
        )
                build_buffer_appendf_("%s/%s", src_dir, 2 + target);
#endif
        else if (target == strstr(target, "../")
#ifdef _WIN32
              || target == strstr(target, "..\\")
        )
                build_buffer_appendf_("%s\\%s", src_dir, target);
#else
        )
                build_buffer_appendf_("%s/%s", src_dir, target);
#endif
        else
                build_buffer_appendf_("%s", target);
#ifdef _WIN32
        for (; i != build_buffer_.count; ++i)
                if ('/' == build_buffer_.data[i])
                        build_buffer_.data[i] = '\\';
#endif
}

static inline time_t build_last_write (
        char const *restrict target,
        char const *restrict src_dir)
{
        time_t result = 0;
        build_full_path_(target, src_dir);
#ifdef _WIN32
        struct _stat st;
        if (_stat(build_buffer_.data, &st)) {
#else
        struct stat st;
        if (stat(build_buffer_.data, &st)) {
#endif
                perror(target);
                goto error;
        }
        result = st.st_mtime;
error:
        build_buffer_.count = 0;
        return result;
}

static inline int build_up_to_date (
        char const        *restrict target,
        char const        *restrict src_dir,
        char const *const *restrict deps)
{
        time_t const target_time = build_last_write(target, src_dir);
        for (char const *const *it = deps; NULL != *it; ++it)
                if (target_time < build_last_write(*it, src_dir))
                        return 0;
        return 1;
}

static inline void build_base_ (
        char const        *restrict target,
        char const        *restrict src_dir,
        char const *const *restrict flags,
        char const *const *restrict inc_dirs,
        char const *const *restrict srcs,
        char const *const *restrict lib_dirs,
        char const *const *restrict libs,
        int                         is_msvc,
        int                         is_silent)
{
        if (NULL != flags)
                for (char const *const *it = flags; NULL != *it; ++it)
                        build_buffer_appendf_(" %s", *it);
        if (NULL != inc_dirs)
                for (char const *const *it = inc_dirs; NULL != *it; ++it) {
                        if (is_msvc)
                                build_buffer_appendf_(" /I\"");
                        else
                                build_buffer_appendf_(" -I\"");
                        build_full_path_(*it, src_dir);
                        build_buffer_appendf_("\"");
                }
        if (NULL != srcs)
                for (char const *const *it = srcs; NULL != *it; ++it) {
                        build_buffer_appendf_(" \"");
                        build_full_path_(*it, src_dir);
                        build_buffer_appendf_("\"");
                }
        if (is_msvc)
                build_buffer_appendf_(" /link");
        if (NULL != lib_dirs)
                for (char const *const *it = lib_dirs; NULL != *it; ++it) {
                        if (is_msvc)
                                build_buffer_appendf_(" /LIBPATH:\"");
                        else
                                build_buffer_appendf_(" -L\"");
                        build_full_path_(*it, src_dir);
                        build_buffer_appendf_("\"");
                }
        if (NULL != libs)
                for (char const *const *it = libs; NULL != *it; ++it) {
                        if (!is_msvc && *it == strstr(*it, "-l")) {
                                if (':' != (*it)[2])
                                        build_buffer_appendf_(" -l\"%s\"", 2 + *it);
                                else
                                        build_buffer_appendf_(" -l:\"%s\"", 3 + *it);
                                continue;
                        }
                        build_buffer_appendf_(" \"");
                        build_full_path_(*it, src_dir);
                        build_buffer_appendf_("\"");
                }
#ifdef _WIN32
        build_buffer_appendf_("\"");
#endif
        if (!is_silent) {
        #ifdef _WIN32
                HANDLE const winout = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(winout, 3);
                printf("%s\n| ", target);
                SetConsoleTextAttribute(winout, 5);
                puts(build_buffer_.data);
                SetConsoleTextAttribute(winout, 7);
        #else
                printf("\033[36m%s\n| \033[35m%s\033[0m\n", target, build_buffer_.data);
        #endif
        }
}

static inline int build_lib(char const *restrict target, struct build_lib args)
{
        if (NULL != args.deps)
                if (build_up_to_date(target, args.src_dir, args.deps))
                        return EXIT_SUCCESS;
        if (NULL == args.compiler)
                args.compiler = "cc";
        else if (0 == strcmp("cl", args.compiler))
                args.is_msvc = 1;
#ifdef _WIN32
        build_buffer_appendf_("\"");
#endif
        build_buffer_appendf_("\"");
        build_full_path_(args.compiler, args.src_dir);
        build_buffer_appendf_("\" ");
        if (args.is_msvc)
                build_buffer_appendf_("/c /Fo\"");
        else
                build_buffer_appendf_("-c -o\"");
        build_full_path_(target, args.src_dir);
        build_buffer_appendf_("\"");
        if (args.is_msvc)
                if (args.is_static)
                        build_buffer_appendf_(" /MT");
                else
                        build_buffer_appendf_(" /MD");
        else
                if (args.is_static)
                        build_buffer_appendf_(" -static");
                else
                        build_buffer_appendf_(" -shared");
        build_base_(target, args.src_dir, args.flags,
                args.inc_dirs, args.srcs,
                args.lib_dirs, args.libs,
                args.is_msvc,  args.is_silent);
        build_buffer_.count = 0;
        return system(build_buffer_.data);
}

static inline int build_exe(char const *restrict target, struct build_exe args)
{
        if (NULL != args.deps)
                if (build_up_to_date(target, args.src_dir, args.deps))
                        return EXIT_SUCCESS;
        if (NULL == args.compiler)
                args.compiler = "cc";
        else if (0 == strcmp("cl", args.compiler))
                args.is_msvc = 1;
#ifdef _WIN32
        build_buffer_appendf_("\"");
#endif
        build_buffer_appendf_("\"");
        build_full_path_(args.compiler, args.src_dir);
        build_buffer_appendf_("\" ");
        if (args.is_msvc)
                build_buffer_appendf_("/Fe\"");
        else
                build_buffer_appendf_("-o\"");
        build_full_path_(target, args.src_dir);
        build_buffer_appendf_("\"");
        if (args.is_msvc)
                if (args.is_static)
                        build_buffer_appendf_(" /MT");
                else
                        build_buffer_appendf_(" /MD");
        else if (args.is_static)
                build_buffer_appendf_(" -static");
        build_base_(target, args.src_dir, args.flags,
                args.inc_dirs, args.srcs,
                args.lib_dirs, args.libs,
                args.is_msvc,  args.is_silent);
        build_buffer_.count = 0;
        return system(build_buffer_.data);
}

static inline int build_refresh (
        char const *restrict target,
        struct build_exe     args)
{
        if (build_up_to_date(target, args.src_dir, args.deps))
                return EXIT_SUCCESS;
        args.deps = NULL;
        remove(        ".build" BUILD_EXTENSION);
        rename(target, ".build" BUILD_EXTENSION);
        int const status = build_exe(target, args);
        if (status)
                return status;
        build_buffer_appendf_("\"");
        build_full_path_(target, args.src_dir);
        build_buffer_appendf_("\"");
        if (!args.is_silent) {
        #ifdef _WIN32
                HANDLE const winout = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(winout, 3);
                printf("%s\n| ", target);
                SetConsoleTextAttribute(winout, 5);
                puts(build_buffer_.data);
                SetConsoleTextAttribute(winout, 7);
        #else
                printf("\033[36m%s\n| \033[35m%s\033[0m\n", target, build_buffer_.data);
        #endif
        }
        build_buffer_.count = 0;
        system(build_buffer_.data);
        return EXIT_FAILURE;
}

#ifdef BUILD_IMPLEMENTATION
        struct build_buffer_ build_buffer_;
#endif

#endif
