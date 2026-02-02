cmake_minimum_required(VERSION 3.10)
project(PipeClient)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compile definitions
if(UNIX)
    add_definitions(-DPIPECLIENT_EXPORTS)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/jsonCpp
)

# Source files
set(SOURCES
    PipeClient.cpp
    PipeClient.h
    app.h
    dllmain.cpp
    pch.cpp
    pch.h
    framework.h
    jsonCpp/jsoncpp.cpp
    jsonCpp/json.h
)

# Create shared library
if(UNIX)
    # Check for uuid library
    find_library(UUID_LIBRARY uuid)
    include(CheckIncludeFile)
    check_include_file(uuid/uuid.h HAVE_UUID_UUID_H)
    
    add_library(PipeClient SHARED ${SOURCES})
    set_target_properties(PipeClient PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        OUTPUT_NAME "PipeClient"
    )
    
    # Link libraries for Linux
    target_link_libraries(PipeClient pthread)
    
    # Link uuid library if available, otherwise use fallback implementation
    if(UUID_LIBRARY AND HAVE_UUID_UUID_H)
        target_link_libraries(PipeClient ${UUID_LIBRARY})
        target_compile_definitions(PipeClient PRIVATE USE_UUID_LIB=1)
    else()
        target_compile_definitions(PipeClient PRIVATE USE_UUID_LIB=0)
        message(WARNING "uuid library not found. Using fallback UUID implementation. Install uuid-dev for better UUID generation: sudo apt-get install uuid-dev")
    endif()
    
    # Set RPATH to help find libraries (optional, for better portability)
    # set_target_properties(PipeClient PROPERTIES
    #     INSTALL_RPATH_USE_LINK_PATH TRUE
    # )
    
    # Install library
    install(TARGETS PipeClient
        LIBRARY DESTINATION lib
    )
else()
    # Windows build (if needed)
    add_library(PipeClient SHARED ${SOURCES})
    set_target_properties(PipeClient PROPERTIES
        OUTPUT_NAME "PipeClient"
    )
endif()

