# share/cmake/CommonDoxy.cmake

# Modified from Barthélémy von Haller's  github.com/Barthelemy/CppProjectTemplate

# STMMI_TARGET_LIB              The library name. Example: stmm-input-gtk-xi
# STMMI_TARGET_LIB_VERSION      The library version. Example: 0.1.0
# STMMI_INCLUDE_LIBS            The other libraries that should be added to the docu.
#                               Example: "stmm-input-ev;stmm-input"
function(CreateLibDoxy STMMI_TARGET_LIB  STMMI_TARGET_LIB_VERSION  STMMI_INCLUDE_LIBS)

    # Add an option for the user to enable or not the documentation generation every time we compile.
    # Either set it on the command line or use cmake-gui. It is an advanced option.
    option(BUILD_DOCS "Build doxygen documentation for ${STMMI_TARGET_LIB}" OFF)
    mark_as_advanced(BUILD_DOCS)

    option(BUILD_DOCS_WARNINGS_TO_LOG_FILE "Doxygen warnings for ${STMMI_TARGET_LIB} written to log file" OFF)
    mark_as_advanced(BUILD_DOCS_WARNINGS_TO_LOG_FILE)

#message(STATUS "CreateLibDoxy STMMI_TARGET_LIB            ${STMMI_TARGET_LIB}")
#message(STATUS "CreateLibDoxy STMMI_TARGET_LIB_VERSION    ${STMMI_TARGET_LIB_VERSION}")
#message(STATUS "CreateLibDoxy STMMI_INCLUDE_LIBS          ${STMMI_INCLUDE_LIBS}")

    set(STMMI_DOC_DIRS_AND_FILES)

    list(LENGTH STMMI_INCLUDE_LIBS STMMI_INCLUDE_LIBS_LEN)
#message(STATUS "CreateLibDoxy STMMI_INCLUDE_LIBS_LEN      ${STMMI_INCLUDE_LIBS_LEN}")

    if (STMMI_INCLUDE_LIBS_LEN GREATER 0)
        set(STMMI_INCLUDE_LIBS_TEMP ${STMMI_INCLUDE_LIBS})
        string(REPLACE ";" ", " STMMI_INCLUDE_LIBS_TEMP "${STMMI_INCLUDE_LIBS_TEMP}")
        #
        option(BUILD_DOCS_INCLUDE_LIBS "Include the ${STMMI_INCLUDE_LIBS_TEMP} headers into doxygen documentation" ON)
        mark_as_advanced(BUILD_DOCS_INCLUDE_LIBS)

        if (BUILD_DOCS AND BUILD_DOCS_INCLUDE_LIBS)
            foreach (STMMI_INCLUDE_CUR_LIB  ${STMMI_INCLUDE_LIBS})
                # ensure the search is really repeated for the current libraries
                # by setting the result variable to -NOTFOUND
                set(STMMITEMPCURLIB "STMMITEMPCURLIB-NOTFOUND")
                find_file(STMMITEMPCURLIB  "${STMMI_INCLUDE_CUR_LIB}"
                            NO_CMAKE_PATH  NO_CMAKE_ENVIRONMENT_PATH)
                if ("${STMMITEMPCURLIB}" STREQUAL "STMMITEMPCURLIB-NOTFOUND")
                    message(FATAL_ERROR "Couldn't find include directory '${STMMI_INCLUDE_CUR_LIB}'")
                else ("${STMMITEMPCURLIB}" STREQUAL "STMMITEMPCURLIB-NOTFOUND")

#message(STATUS "CreateLibDoxy STMMITEMPCURLIB      ${STMMITEMPCURLIB}")
                    list(APPEND STMMI_DOC_DIRS_AND_FILES   ${STMMITEMPCURLIB})

                endif ("${STMMITEMPCURLIB}" STREQUAL "STMMITEMPCURLIB-NOTFOUND")
                mark_as_advanced(STMMITEMPCURLIB)
            endforeach (STMMI_INCLUDE_CUR_LIB  ${STMMI_INCLUDE_LIBS})
        endif (BUILD_DOCS AND BUILD_DOCS_INCLUDE_LIBS)
    endif (STMMI_INCLUDE_LIBS_LEN GREATER 0)

    if (BUILD_DOCS)
        list(APPEND STMMI_DOC_DIRS_AND_FILES
                ${PROJECT_SOURCE_DIR}/include
                ${PROJECT_SOURCE_DIR}/doc
                ${PROJECT_SOURCE_DIR}/README.md
                )
        string(REPLACE ";" " " STMMI_DOC_DIRS_AND_FILES "${STMMI_DOC_DIRS_AND_FILES}")

#message(STATUS "CreateLibDoxy STMMI_DOC_DIRS_AND_FILES      ${STMMI_DOC_DIRS_AND_FILES}")

        if (BUILD_DOCS_WARNINGS_TO_LOG_FILE)
            set(STMMI_DOXY_WARNING_LOG_FILE ${CMAKE_BINARY_DIR}/lib${STMMI_TARGET_LIB}_doxy.log)
        else (BUILD_DOCS_WARNINGS_TO_LOG_FILE)
            set(STMMI_DOXY_WARNING_LOG_FILE)
        endif (BUILD_DOCS_WARNINGS_TO_LOG_FILE)

        # Configure the doxygen config file with current settings:
        configure_file(${PROJECT_SOURCE_DIR}/../share/doc/docu-config.doxygen.in  ${CMAKE_BINARY_DIR}/docu-config.doxygen @ONLY)

        add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/docu-config.doxygen
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Doxygen: generating API documentation for ${STMMI_TARGET_LIB}" VERBATIM)

        make_directory(${CMAKE_BINARY_DIR}/html) # needed for install

        install(DIRECTORY ${CMAKE_BINARY_DIR}/html DESTINATION share/doc/${STMMI_TARGET_LIB} COMPONENT doc)
    endif (BUILD_DOCS)

endfunction(CreateLibDoxy STMMI_TARGET_LIB  STMMI_TARGET_LIB_VERSION  STMMI_INCLUDE_LIBS)
