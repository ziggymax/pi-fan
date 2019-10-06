cmake_minimum_required(VERSION 3.13)
project(pi-fan)

# General build directives and options
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_BUILD_TYPE Release)

# ----------------------

set(INCL_LIST
    include/bcm2835.h
    include/utils.h
)

set(SRC_LIST
    src/main.c
    src/bcm2835.c
    src/utils.c
)

# ----------------------

add_executable(${PROJECT_NAME} "")

target_sources(
    ${PROJECT_NAME} PRIVATE
    ${SRC_LIST}
    ${INCL_LIST}
)

### Find/setup dependencies
#find_library(FMTLIB NAMES fmt REQUIRED)
#find_package(Threads REQUIRED)

### Include directories
target_include_directories(${PROJECT_NAME} PUBLIC include src)

### Setup compile flags and options
# 1) We first have to reset defaults to nothing. Which is silly, but it seems
#   to be necessary to override CMake defaults in step 2.
set(CMAKE_C_FLAGS_RELEASE "")
set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "")
set(CMAKE_C_FLAGS_MINSIZEREL "")
# 2) Then set the options we actually want
set(COMPILE_OPTIONS_COMMON -Wall -Wextra -Werror=format-security -fstack-protector-strong -march=armv8-a -mfloat-abi=hard -mfpu=neon-fp-armv8)
set(COMPILE_OPTIONS_REL ${COMPILE_OPTIONS_COMMON} -O2 -DNDEBUG -Wp,-D_FORTIFY_SOURCE=2)
set(COMPILE_OPTIONS_DEB ${COMPILE_OPTIONS_COMMON} -g -Og)
set(COMPILE_OPTIONS_RDB ${COMPILE_OPTIONS_COMMON} -g -Og -DNDEBUG -Wp,-D_FORTIFY_SOURCE=2)
set(COMPILE_OPTIONS_MIN ${COMPILE_OPTIONS_COMMON} -Os -DNDEBUG -Wp,-D_FORTIFY_SOURCE=2)
# 3) Map compile options based on configuration.
target_compile_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:RELEASE>:${COMPILE_OPTIONS_REL}>)
target_compile_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:DEBUG>:${COMPILE_OPTIONS_DEB}>)
target_compile_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:RELWITHDEBINFO>:${COMPILE_OPTIONS_RDB}>)
target_compile_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:MINSIZEREL>:${COMPILE_OPTIONS_MIN}>)
# 4) Set compile flags across packages.
target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)

### Specify use of -pthread compiler/linker flags.
#set(THREADS_PREFER_PTHREAD_FLAG TRUE)

### Library/link options
#target_link_libraries(${PROJECT_NAME} ${FMTLIB} Threads::Threads)
