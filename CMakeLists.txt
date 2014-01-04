CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_FLAGS "-g -Wall")
PROJECT(net)

# Optional packages
OPTION(ENABLE_SSL "Enable SSL (requires OpenSSL)" ON)
IF(ENABLE_SSL)
    FIND_PACKAGE(OpenSSL REQUIRED)
    INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
    SET(EXTRA_LIBRARIES ${OPENSSL_LIBRARIES} ${EXTRA_LIBRARIES})
ENDIF()

# Sets up project-wide include dirs
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}")

# Recursively builds components
ADD_SUBDIRECTORY(sockets)

# Testing
OPTION(BUILD_TESTSUITE "Build the test suite (requires CppUnit)" OFF)
IF(BUILD_TESTSUITE)
    FIND_PACKAGE(CppUnit REQUIRED)
    INCLUDE_DIRECTORIES(${CPPUNIT_INCLUDE_DIRS})

    ADD_EXECUTABLE(test_sockets EXCLUDE_FROM_ALL test_sockets.cpp)
    TARGET_LINK_LIBRARIES(test_sockets sockets ${CPPUNIT_LIBRARIES})

    # Creates the 'make test' target
    ADD_CUSTOM_TARGET(
        test test_sockets
        DEPENDS test_sockets)
ENDIF()
