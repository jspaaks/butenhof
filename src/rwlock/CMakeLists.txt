add_executable(tgt_exe_rwlock)

set_property(TARGET tgt_exe_rwlock PROPERTY OUTPUT_NAME rwlock)

target_compile_definitions(
    tgt_exe_rwlock
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_features(
    tgt_exe_rwlock
    PRIVATE
        c_std_23
)

target_compile_options(
    tgt_exe_rwlock
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
    tgt_exe_rwlock
    PRIVATE
        pthread
)

target_link_options(
    tgt_exe_rwlock
    PRIVATE
        -pie
)

target_sources(
    tgt_exe_rwlock
    PRIVATE
        main.c
        rwlock.c
)

install(TARGETS tgt_exe_rwlock)
