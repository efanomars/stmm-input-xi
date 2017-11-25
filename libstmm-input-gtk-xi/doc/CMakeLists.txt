# stmm-input-gtk-xi: doc/CMakeLists.txt

include(FindDoxygen)

if (DOXYGEN_FOUND)
    if (NOT DOXYGEN_DOT_FOUND) 
        message(WARNING "Graphviz doesn't seem to be installed. Doxygen will not be able to generate graphs. Consider installing this package.")
    endif(NOT DOXYGEN_DOT_FOUND) 

    include(CommonDoxy)
    CreateLibDoxy(stmm-input-gtk-xi  ${STMM_INPUT_GTK_XI_VERSION}  "stmm-input-gtk;stmm-input-ev;stmm-input-base;stmm-input")
else(DOXYGEN_FOUND)
    add_custom_target(doc "Doxygen not found")
endif(DOXYGEN_FOUND)
