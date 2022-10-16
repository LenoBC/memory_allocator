cmake_minimum_required(VERSION 3.17)
project(memory_allocator C)

set(CMAKE_C_STANDARD 11)

add_compile_options(
        "-ggdb3"
        "-std=c11"
        "-xc"
        "-pedantic"
        "-Werror"
        "-Wall"
        "-Wextra"
        "-fmax-errors=5"
        "-Werror=vla"
        "-Wno-error=unused-parameter"
        "-Wno-error=parentheses"
        "-Wno-parentheses"
        "-D_GNU_SOURCE"
        "-D_TEST_BOOTSTRAP"
        "-DINSIDE_DANTE"
)


add_link_options(
        "-ggdb3"
        "-fuse-ld=gold"
        "-Wl,-Map=main.map"
        "-Wl,-cref"
        "-Wl,-wrap,main"
)


add_executable(memory_allocator
        main.c # Nazwa głównego pliku, z funkcją main
        heap.h heap.c
        custom_unistd.h memmanager.c
        unit_test_v2.c
        unit_helper_v2.c unit_helper_v2.h
        tested_declarations.h
        rdebug.c rdebug.h
        memmanager.c custom_unistd.h
        )


target_link_libraries(memory_allocator m pthread)