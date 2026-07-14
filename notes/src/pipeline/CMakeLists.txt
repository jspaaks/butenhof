add_executable(tgt_exe_pipeline)

set_property(TARGET tgt_exe_pipeline PROPERTY OUTPUT_NAME pipeline)

target_compile_definitions(
    tgt_exe_pipeline
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_features(
    tgt_exe_pipeline
    PRIVATE
        c_std_23
)

target_compile_options(
    tgt_exe_pipeline
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
    tgt_exe_pipeline
    PRIVATE
        pthread
)

target_link_options(
    tgt_exe_pipeline
    PRIVATE
        -pie
)

target_sources(
    tgt_exe_pipeline
    PRIVATE
        main.c
        queue.c
)

install(TARGETS tgt_exe_pipeline)
