# SpriteStep version used by the native/SDL shell.
#
# The pre-2.0 Android tree stored this in app/build.gradle.kts. The current architecture no longer
# carries the Android application, so the desktop/PortMaster shell must not depend on that removed
# tree just to configure. When the legacy Gradle file exists we preserve its version; otherwise the
# current development line uses an explicit development version.

set(PT_VERSION_FILE "${CMAKE_CURRENT_LIST_DIR}/../../app/build.gradle.kts")
if(EXISTS "${PT_VERSION_FILE}")
    file(READ "${PT_VERSION_FILE}" PT_GRADLE)
    if(NOT PT_GRADLE MATCHES "versionName[ \\t]*=[ \\t]*\"([0-9]+)\\.([0-9]+)\\.([0-9]+)\"")
        message(FATAL_ERROR
                "could not find versionName in app/build.gradle.kts - if its syntax changed, update "
                "the regex in native/cmake/pt_version.cmake")
    endif()
    set(PT_VERSION       "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
    set(PT_VERSION_COMMA "${CMAKE_MATCH_1},${CMAKE_MATCH_2},${CMAKE_MATCH_3},0")
    message(STATUS "SPRITESTEP ${PT_VERSION} (from app/build.gradle.kts)")
else()
    set(PT_VERSION "2.0.0-dev")
    set(PT_VERSION_COMMA "2,0,0,0")
    message(STATUS "SPRITESTEP ${PT_VERSION} (development shell build)")
endif()
