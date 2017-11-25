# share/cmake/CommonUtil.cmake

# CheckBinaryNotSourceTree    Check not building in the source tree
#
function(CheckBinaryNotSourceTree)
    if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
        message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the ${CMAKE_PROJECT_NAME} source code and call cmake from there")
    endif()
endfunction(CheckBinaryNotSourceTree)


# CheckBuildType              Check CMAKE_BUILD_TYPE is valid
#
function(CheckBuildType)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(STMMI_BUILD_TYPE "DEBUG" PARENT_SCOPE)
    elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        set(STMMI_BUILD_TYPE "RELEASE" PARENT_SCOPE)
    elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
        set(STMMI_BUILD_TYPE "MINSIZEREL" PARENT_SCOPE)
    elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
        set(STMMI_BUILD_TYPE "RELWITHDEBINFO" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Wrong CMAKE_BUILD_TYPE, set to either Debug, Release, MinSizeRel or RelWithDebInfo")
    endif()
    #
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON    PARENT_SCOPE)
    #
endfunction(CheckBuildType)


# DefineSharedLibOption       Define cmake option BUILD_SHARED_LIBS
#
function(DefineSharedLibOption)
    option(BUILD_SHARED_LIBS "Build shared library ${CMAKE_PROJECT_NAME}" ON)
    mark_as_advanced(BUILD_SHARED_LIBS)
endfunction(DefineSharedLibOption)

# DefineCommonOptions         Define cmake options for all subprojects
#
function(DefineCommonOptions)
    option(BUILD_WITH_SANITIZE "Build with sanitize=address (Debug only!)" OFF)
    mark_as_advanced(BUILD_WITH_SANITIZE)
endfunction(DefineCommonOptions)


# DefineCommonCompileOptions  Define compile options for all subprojects
#
function(DefineCommonCompileOptions)
    # Compiler option also used for googletest
    string(STRIP "${CMAKE_CXX_FLAGS} -std=c++14" CMAKE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
    if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
        if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
            if (BUILD_WITH_SANITIZE)
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address" PARENT_SCOPE)
            endif()
        endif()
    endif()
endfunction(DefineCommonCompileOptions)

# DefineTargetInterfaceCompileOptions    Define compile options for a header only library target
#
# Parameters:
# STMMI_TARGET   The (library) target
#
function(DefineTargetInterfaceCompileOptions STMMI_TARGET)
    DefineTargetCompileOptionsType(${STMMI_TARGET} ON OFF)
endfunction(DefineTargetInterfaceCompileOptions)

# DefineTargetPublicCompileOptions       Define compile options for a normal target
#
# Parameters:
# STMMI_TARGET   The (library) target
#
function(DefineTargetPublicCompileOptions STMMI_TARGET)
    DefineTargetCompileOptionsType(${STMMI_TARGET} OFF OFF)
endfunction(DefineTargetPublicCompileOptions)

# DefineTestTargetPublicCompileOptions       Define compile options for a test target
#
# Parameters:
# STMMI_TARGET   The (library) target
#
function(DefineTestTargetPublicCompileOptions STMMI_TARGET)
    DefineTargetCompileOptionsType(${STMMI_TARGET} OFF ON)
endfunction(DefineTestTargetPublicCompileOptions)

# private:
function(DefineTargetCompileOptionsType STMMI_TARGET STMMI_INTERFACE_TYPE STMMI_USES_GTEST)
    # STMMI_COMPILE_WARNINGS holds target specific flags
    set(STMMI_COMPILE_WARNINGS "${CMAKE_CXX_FLAGS}")
    set(STMMI_PRIVATE_COMPILE_WARNINGS "")
    if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
        set(STMMI_COMPILE_WARNINGS "${STMMI_COMPILE_WARNINGS} -Wall -Wextra \
-pedantic-errors -Wmissing-include-dirs -Winit-self \
-Woverloaded-virtual -Wsign-promo")
        if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
            set(STMMI_COMPILE_WARNINGS "${STMMI_COMPILE_WARNINGS} -ggdb")
        endif()
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        # no idea if this really works
        set(STMMI_COMPILE_WARNINGS "${STMMI_COMPILE_WARNINGS} /W2 /EHsc") # /WX warnings as errors
    endif()
    if (NOT STMMI_USES_GTEST)
        set(STMMI_PRIVATE_COMPILE_WARNINGS "${STMMI_PRIVATE_COMPILE_WARNINGS} $ENV{STMM_CPP_OPTIONS}")
    endif()
    #
    string(STRIP "${STMMI_COMPILE_WARNINGS}" STMMI_COMPILE_WARNINGS)
    if (NOT ("${STMMI_COMPILE_WARNINGS}" STREQUAL ""))
        string(REPLACE " " ";" REPLACED_FLAGS ${STMMI_COMPILE_WARNINGS})
    endif()
    if (STMMI_INTERFACE_TYPE)
        target_compile_options(${STMMI_TARGET} INTERFACE ${REPLACED_FLAGS})
    else()
        target_compile_options(${STMMI_TARGET} PUBLIC ${REPLACED_FLAGS})
        string(STRIP "${STMMI_PRIVATE_COMPILE_WARNINGS}" STMMI_PRIVATE_COMPILE_WARNINGS)
        if (NOT ("${STMMI_PRIVATE_COMPILE_WARNINGS}" STREQUAL ""))
            string(REPLACE " " ";" REPLACED_FLAGS ${STMMI_PRIVATE_COMPILE_WARNINGS})
            target_compile_options(${STMMI_TARGET} PRIVATE ${REPLACED_FLAGS})
        endif()
    endif()
endfunction(DefineTargetCompileOptionsType)
