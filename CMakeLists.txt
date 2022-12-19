cmake_minimum_required(VERSION 3.20)
project(pi-fan-controller)

# General build directives and options
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_BUILD_TYPE Release)

set(INCL_LIST
    include/bcm2835.h
    include/utils.h
)

set(SRC_LIST
    src/main.c
    src/bcm2835.c
    src/utils.c
)

set(EXECUTABLE_TARGET pi-fan)
add_custom_target(${PROJECT_NAME} ALL)
add_executable(${EXECUTABLE_TARGET} "")

target_sources(
    ${EXECUTABLE_TARGET} PRIVATE
    ${SRC_LIST}
    ${INCL_LIST}
)

# Include directories
target_include_directories(${EXECUTABLE_TARGET} PUBLIC include src)

# We first have to reset defaults to nothing. Which is silly, but it seems
# to be necessary to override CMake defaults in step 2.
set(CMAKE_C_FLAGS_RELEASE "")
set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "")
set(CMAKE_C_FLAGS_MINSIZEREL "")

# Set config dependent options
set(COMPILE_OPTIONS_COMMON -Wall -Wextra -Werror=format-security -fstack-protector-strong -mcpu=cortex-a72)
set(COMPILE_OPTIONS_REL ${COMPILE_OPTIONS_COMMON} -O2 -DNDEBUG -Wp,-D_FORTIFY_SOURCE=2)
set(COMPILE_OPTIONS_DEB ${COMPILE_OPTIONS_COMMON} -g -Og)
set(COMPILE_OPTIONS_RDB ${COMPILE_OPTIONS_COMMON} -g -Og -DNDEBUG -Wp,-D_FORTIFY_SOURCE=2)
set(COMPILE_OPTIONS_MIN ${COMPILE_OPTIONS_COMMON} -Os -DNDEBUG -Wp,-D_FORTIFY_SOURCE=2)

# Map compile options based on configuration.
target_compile_options(${EXECUTABLE_TARGET} PUBLIC $<$<CONFIG:RELEASE>:${COMPILE_OPTIONS_REL}>)
target_compile_options(${EXECUTABLE_TARGET} PUBLIC $<$<CONFIG:DEBUG>:${COMPILE_OPTIONS_DEB}>)
target_compile_options(${EXECUTABLE_TARGET} PUBLIC $<$<CONFIG:RELWITHDEBINFO>:${COMPILE_OPTIONS_RDB}>)
target_compile_options(${EXECUTABLE_TARGET} PUBLIC $<$<CONFIG:MINSIZEREL>:${COMPILE_OPTIONS_MIN}>)

# Set compile flags across packages.
target_compile_features(${EXECUTABLE_TARGET} PUBLIC c_std_11)

# Add extra pseudo target to strip executable for symbols
add_custom_target(
    strip_executable ALL
    COMMAND /opt/pi-cross-compile/aarch64-rpi3-linux-gnu/bin/aarch64-rpi3-linux-gnu-strip pi-fan
)

# Add dependencies to all sub targets
add_dependencies(${PROJECT_NAME} ${EXECUTABLE_TARGET} strip_executable)
add_dependencies(strip_executable ${EXECUTABLE_TARGET})
