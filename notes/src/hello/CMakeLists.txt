add_executable(tgt_exe_hello)

set_property(TARGET tgt_exe_hello PROPERTY OUTPUT_NAME hello)

target_compile_definitions(
    tgt_exe_hello
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_features(
    tgt_exe_hello
    PRIVATE
        c_std_23
)

target_compile_options(
    tgt_exe_hello
    PRIVATE
        -Wall
        -Wextra
        -pedantic
        -fPIE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Release>:-Werror>
)

target_link_libraries(
    tgt_exe_hello
    PRIVATE
        pthread
)

target_link_options(
    tgt_exe_hello
    PRIVATE
        -pie
)

target_sources(
    tgt_exe_hello
    PRIVATE
        main.c
)

install(TARGETS tgt_exe_hello)
