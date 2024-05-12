cmake_minimum_required(VERSION 3.21)
project(aviutl_ShowLimit)

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
    vendor/aviutl_exedit_sdk
)
target_link_libraries(ShowLimit comctl32 bcrypt)
target_compile_options(ShowLimit
    PRIVATE
        "$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>"
        "$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:shift_jis>"
)
set_target_properties(ShowLimit PROPERTIES SUFFIX .auf)
