#################################################
# Controls various project wide sanitizer options
#################################################

set(USE_SANITIZER "Off" CACHE STRING "Sanitizer mode to enable")
set_property(CACHE USE_SANITIZER PROPERTY STRINGS
             Off Address Thread Undefined)

string(TOLOWER "${USE_SANITIZER}" USE_SANITIZERS_LOWER)

if(NOT ${USE_SANITIZERS_LOWER} MATCHES "off")
    # Check we have a supported compiler
    if(WIN32)
        message(FATAL_ERROR "Windows does not support sanitizers")
    endif()

    if(CMAKE_COMPILER_IS_GNUCXX AND (NOT CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8))
        message(FATAL_ERROR "GCC 7 and below do not support sanitizers")
    endif()

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU"
        OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    # Append path of libAsan to ${ASAN_SO_PATH}
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libasan.so
                    OUTPUT_VARIABLE ASAN_SO_PATH
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    # Check and warn if we are not in a mode without debug symbols
    string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type_lower)
    if("${build_type_lower}" MATCHES "release" OR "${build_type_lower}" MATCHES "minsizerel" )
        message(WARNING "You are running address sanitizers without debug information, try RelWithDebInfo")

    elseif(${build_type_lower} MATCHES "relwithdebinfo")
        # RelWithDebug runs with -o2 which performs inlining reducing the usefulness
        message("Replacing -O2 flag with -O1 to preserve stack trace on sanitizers")
        string(REPLACE "-O2" "-O1" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
        string(REPLACE "-O2" "-O1" CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})

        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} CACHE STRING "" FORCE)
        set(CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} CACHE STRING "" FORCE)

        add_compile_options(-fno-omit-frame-pointer -fno-optimize-sibling-calls)
    endif()
endif()

# Allow all instrumented code to continue beyond the first error
add_compile_options($<$<NOT:$<STREQUAL:$<LOWER_CASE:"${USE_SANITIZER}">,"off">>:-fsanitize-recover=all>)

set( _SUPPRESSIONS_DIR "${CMAKE_SOURCE_DIR}/tools/Sanitizer" )

# Address
add_compile_options(
    $<$<STREQUAL:$<LOWER_CASE:"${USE_SANITIZER}">,"address">:-fsanitize=address>)
add_link_options(
    $<$<STREQUAL:$<LOWER_CASE:"${USE_SANITIZER}">,"address">:-fsanitize=address>)
if (${USE_SANITIZERS_LOWER} STREQUAL "address")
    set(ASAN_ENV "suppressions=${_SUPPRESSIONS_DIR}/Address.supp:detect_stack_use_after_return=true")
    set(LSAN_ENV "suppressions=${_SUPPRESSIONS_DIR}/Leak.supp")
endif()

# Thread
add_compile_options(
    $<$<STREQUAL:$<LOWER_CASE:"${USE_SANITIZER}">,"thread">:-fsanitize=thread>)
add_link_options(
    $<$<STREQUAL:$<LOWER_CASE:"${USE_SANITIZER}">,"thread">:-fsanitize=thread>)

# Undefined
# RTTI information is not exported for some classes causing the
# linker to fail whilst adding vptr instrumentation
add_compile_options(
    $<$<STREQUAL:$<LOWER_CASE:"${USE_SANITIZER}">,"undefined">:-fsanitize=undefined>
    $<$<STREQUAL:$<LOWER_CASE:"${USE_SANITIZER}">,"undefined">:-fno-sanitize=vptr>)

add_link_options(
    $<$<STREQUAL:$<LOWER_CASE:"${USE_SANITIZER}">,"undefined">:-fsanitize=undefined>)
