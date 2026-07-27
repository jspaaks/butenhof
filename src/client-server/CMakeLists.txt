add_executable(tgt_exe_client_server)

set_property(TARGET tgt_exe_client_server PROPERTY OUTPUT_NAME client-server)

target_compile_definitions(
    tgt_exe_client_server
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_features(
    tgt_exe_client_server
    PRIVATE
        c_std_23
)

target_compile_options(
    tgt_exe_client_server
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
    tgt_exe_client_server
    PRIVATE
        pthread
)

target_link_options(
    tgt_exe_client_server
    PRIVATE
        -pie
)

target_sources(
    tgt_exe_client_server
    PRIVATE
        main.c
        queue.c
)

install(TARGETS tgt_exe_client_server)
