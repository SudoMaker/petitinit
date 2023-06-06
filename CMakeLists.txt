cmake_minimum_required(VERSION 3.0)
project(petitinit C CXX)

set(CMAKE_CXX_STANDARD 14)

add_compile_options(-g -Os -nodefaultlibs -fno-exceptions)
add_link_options(-g -Os -nodefaultlibs -fno-exceptions)

if (CMAKE_C_COMPILER MATCHES "mips")
    message("-- Optimizing for Ingenic MIPS platform")
    set(CFLAGS_COMMON -march=mips32r2 -mno-interlink-compressed -mno-shared -g -Os
            --param=l1-cache-size=16 --param=l1-cache-line-size=32 --param=l2-cache-size=128
            -ffunction-sections -fdata-sections
            -flto -ffat-lto-objects
            )
    add_compile_options(${CFLAGS_COMMON})
    add_link_options(${CFLAGS_COMMON} -Wl,-flto -Wl,--gc-sections -static -z noexecstack)
endif()

function(add_target NAME)
    file(GLOB_RECURSE BOARD_SOURCE_FILE_LIST "${CMAKE_CURRENT_SOURCE_DIR}/target/${NAME}/*.*")
    add_executable(petitinit_${NAME} ${BOARD_SOURCE_FILE_LIST} petitinit.cpp)
    target_link_libraries(petitinit_${NAME} c)
endfunction()

add_target(x1000_nano_som)
add_target(cle)
add_target(loshark_v2.2)

