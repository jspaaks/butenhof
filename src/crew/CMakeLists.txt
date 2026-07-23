add_executable(tgt_exe_crew)

set_property(TARGET tgt_exe_crew PROPERTY OUTPUT_NAME crew)

target_compile_definitions(
    tgt_exe_crew
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_features(
    tgt_exe_crew
    PRIVATE
        c_std_23
)

target_compile_options(
    tgt_exe_crew
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
    tgt_exe_crew
    PRIVATE
        pthread
)

target_link_options(
    tgt_exe_crew
    PRIVATE
        -pie
)

target_sources(
    tgt_exe_crew
    PRIVATE
        main.c
        queue.c
)

install(TARGETS tgt_exe_crew)
