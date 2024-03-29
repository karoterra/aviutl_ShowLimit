cmake_minimum_required(VERSION 3.20)
project(aviutl_ShowLimit)

set(AVIUTL_INCLUDE_DIR "vendor" CACHE PATH "AviUtl Plugin SDK include dir")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ShowLimit SHARED
    src/show_limit.cpp
    src/Sha256Hasher.hpp
    src/Sha256Hasher.cpp
    src/AviUtlProfiler.hpp
    src/AviUtlProfiler.cpp
    src/PluginsOption.hpp
    src/ExEditProfiler.hpp
    src/ExEditProfiler.cpp
    src/NamesBuffer.hpp
    src/ScriptsOption.hpp
    src/LanguagePlugin.hpp
)
target_include_directories(ShowLimit PRIVATE
    ${AVIUTL_INCLUDE_DIR}
    ${AVIUTL_INCLUDE_DIR}/aviutl_exedit_sdk
)
target_link_libraries(ShowLimit comctl32 bcrypt)
set_target_properties(ShowLimit PROPERTIES SUFFIX .auf)
