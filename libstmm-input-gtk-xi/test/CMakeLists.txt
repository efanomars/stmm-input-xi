# stmm-input-gtk-xi: test/CMakeLists.txt

# See   github.com/Barthelemy/CppProjectTemplate

include(CommonTesting)

option(BUILD_TESTING "Build tests" OFF)

if (BUILD_TESTING)
    # Tests dir
    set(STMMI_TEST_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/test")
    # Test sources need to end with .cxx, helper sources with .h .cc
    set(STMMI_GTK_XI_TEST_SOURCES
            ${STMMI_TEST_SOURCES_DIR}/testFloGtkDeviceManager.cxx
            )

    set(STMMI_GTK_XI_TEST_WITH_SOURCES
            ${STMMI_TEST_SOURCES_DIR}/fakeflogtkbackend.h
            ${STMMI_TEST_SOURCES_DIR}/fakeflogtkbackend.cc
            ${STMMI_TEST_SOURCES_DIR}/fakeflogtkwindowdata.h
            ${STMMI_TEST_SOURCES_DIR}/fakeflogtkwindowdata.cc
            ${STMMI_TEST_SOURCES_DIR}/fakeflogtkdevicemanager.h
            ${STMMI_TEST_SOURCES_DIR}/fixtureFloDM.h
            ${STMMI_TEST_SOURCES_DIR}/fixtureGlibApp.h
            ${STMMI_TEST_SOURCES_DIR}/fixturevariantKeyRepeatMode.h
            ${STMMI_TEST_SOURCES_DIR}/fixturevariantEventClasses.h
            )

    TestFiles("${STMMI_GTK_XI_TEST_SOURCES}"
              "${STMMI_SOURCES};${STMMI_GTK_XI_TEST_WITH_SOURCES}"
              "${STMMI_GTK_XI_EXTRA_LIBS}" TRUE)

    include(CTest)
endif()
