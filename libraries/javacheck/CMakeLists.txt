cmake_minimum_required(VERSION 3.15)
project(launcher Java)
find_package(Java 1.8 REQUIRED COMPONENTS Development)

include(UseJava)
set(CMAKE_JAVA_JAR_ENTRY_POINT JavaCheck)
if(Java_VERSION VERSION_GREATER_EQUAL 9)
    set(CMAKE_JAVA_COMPILE_FLAGS --release 8 -Xlint:-options -Xlint:deprecation -Xlint:unchecked)
else()
    set(CMAKE_JAVA_COMPILE_FLAGS -source 8 -target 8 -Xlint:deprecation -Xlint:unchecked)
endif()

set(SRC
    JavaCheck.java
)

add_jar(JavaCheck ${SRC})
install_jar(JavaCheck "${JARS_DEST_DIR}")
