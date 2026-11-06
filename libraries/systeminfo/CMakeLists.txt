project(systeminfo)

find_package(Qt6 COMPONENTS Core REQUIRED)

set(systeminfo_SOURCES
    include/sys.h
    include/distroutils.h
    src/distroutils.cpp)

if (WIN32)
    list(APPEND systeminfo_SOURCES src/sys_win32.cpp)
elseif (UNIX)
    if(APPLE)
        list(APPEND systeminfo_SOURCES src/sys_apple.cpp)
    else()
        list(APPEND systeminfo_SOURCES src/sys_unix.cpp)
    endif()
endif()

add_library(systeminfo STATIC ${systeminfo_SOURCES})
target_link_libraries(systeminfo Qt6::Core Qt6::Gui Qt6::Network ${systeminfo_LIBS})
target_include_directories(systeminfo PUBLIC include)
