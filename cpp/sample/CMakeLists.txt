# CMakeLists.txt for jstester sample application

add_executable (jstester jstester.cpp)

target_link_libraries (jstester LINK_PUBLIC JoystickLibrary)