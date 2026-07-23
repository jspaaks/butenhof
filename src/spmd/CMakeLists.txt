add_executable(tgt_exe_spmd)

set_property(TARGET tgt_exe_spmd PROPERTY OUTPUT_NAME spmd)

target_compile_definitions(
    tgt_exe_spmd
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_features(
    tgt_exe_spmd
    PRIVATE
        c_std_23
)

target_compile_options(
    tgt_exe_spmd
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
    tgt_exe_spmd
    PRIVATE
        pthread
)

target_link_options(
    tgt_exe_spmd
    PRIVATE
        -pie
)

target_sources(
    tgt_exe_spmd
    PRIVATE
        main.c
)

install(TARGETS tgt_exe_spmd)
