# CMakeLists.txt for jstester sample application

add_executable (jstester jstester.cpp ServiceTester.h)

target_link_libraries (jstester LINK_PUBLIC JoystickLibrary)