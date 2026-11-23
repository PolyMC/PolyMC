cmake_minimum_required(VERSION 3.15)
project(launcher Java)
find_package(Java 1.8 REQUIRED COMPONENTS Development)

include(UseJava)
set(CMAKE_JAVA_JAR_ENTRY_POINT org.polymc.EntryPoint)
if(Java_VERSION VERSION_GREATER_EQUAL 9)
    set(CMAKE_JAVA_COMPILE_FLAGS --release 8 -Xlint:-options -Xlint:deprecation -Xlint:unchecked)
else()
    set(CMAKE_JAVA_COMPILE_FLAGS -source 8 -target 8 -Xlint:deprecation -Xlint:unchecked)
endif()

set(SRC
    org/polymc/EntryPoint.java
    org/polymc/Launcher.java
    org/polymc/LauncherFactory.java
    org/polymc/impl/OneSixLauncher.java
    org/polymc/applet/LegacyFrame.java
    org/polymc/exception/ParameterNotFoundException.java
    org/polymc/exception/ParseException.java
    org/polymc/utils/Parameters.java
    org/polymc/utils/Utils.java
    net/minecraft/Launcher.java
    com/mojang/patchy/BlockedServers.java
)
add_jar(NewLaunch ${SRC})
install_jar(NewLaunch "${JARS_DEST_DIR}")

include(GNUInstallDirs)
install(FILES legacy-jar-security.properties
    DESTINATION ${PROP_DEST_DIR})

# needed for local builds
file(COPY_FILE legacy-jar-security.properties
    ${CMAKE_BINARY_DIR}/legacy-jar-security.properties)
