add_executable(tgt_exe_warehouse)

set_property(TARGET tgt_exe_warehouse PROPERTY OUTPUT_NAME warehouse)

target_compile_definitions(
    tgt_exe_warehouse
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_features(
    tgt_exe_warehouse
    PRIVATE
        c_std_23
)

target_compile_options(
    tgt_exe_warehouse
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
    tgt_exe_warehouse
    PRIVATE
        pthread
)

target_link_options(
    tgt_exe_warehouse
    PRIVATE
        -pie
)

target_sources(
    tgt_exe_warehouse
    PRIVATE
        main.c
        stack.c
)

install(TARGETS tgt_exe_warehouse)
