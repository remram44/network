# Write configuration to config.h
set(SOCKETS_VERSION_MAJOR 0)
set(SOCKETS_VERSION_MINOR 1)

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

# Build source list with enabled components
SET(SOCKETS_FILES Socket.cpp TCP.cpp UDP.cpp)
IF(ENABLE_SSL)
    SET(SOCKETS_FILES ${SOCKETS_FILES} SSLSocket.cpp)
ENDIF()

# Build library
ADD_LIBRARY(sockets ${SOCKETS_FILES})
TARGET_LINK_LIBRARIES(sockets ${EXTRA_LIBRARIES})
IF(WIN32)
    TARGET_LINK_LIBRARIES(sockets ws2_32)
ENDIF()
