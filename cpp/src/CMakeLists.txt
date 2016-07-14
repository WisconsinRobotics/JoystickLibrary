find_package(Threads REQUIRED)
set (CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(JOYSTICK_LIBRARY_WIN32_SRC
    joystick.cpp
    joystick_win32.cpp
)

set(JOYSTICK_LIBRARY_LINUX_SRC
    joystick.cpp
    joystick_linux.cpp
)

set(JOYSTICK_LIBRARY_HEADERS
    ${CMAKE_SOURCE_DIR}/include/joystick.h
)

if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
    add_library(JoystickLibrary STATIC ${JOYSTICK_LIBRARY_WIN32_SRC} ${JOYSTICK_LIBRARY_HEADERS})
    
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
    
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-missing-field-initializers -pedantic")
    find_package(PkgConfig)
    pkg_search_module(LIBEVDEV REQUIRED libevdev)
    add_library(JoystickLibrary STATIC ${JOYSTICK_LIBRARY_LINUX_SRC} ${JOYSTICK_LIBRARY_HEADERS})
    target_link_libraries(JoystickLibrary ${LIBEVDEV_LIBRARIES})
    target_include_directories(JoystickLibrary PUBLIC ${LIBEVDEV_INCLUDE_DIRS})
    target_compile_options(JoystickLibrary PUBLIC ${LIBEVDEV_CFLAGS_OTHER})
endif()

target_link_libraries (JoystickLibrary ${CMAKE_THREAD_LIBS_INIT})
