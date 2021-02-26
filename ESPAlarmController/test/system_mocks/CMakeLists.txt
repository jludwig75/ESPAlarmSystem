add_library(system_mocks STATIC
            Arduino.cpp
            FS.cpp
            FileData.cpp
            FileImpl.cpp
            FsImpl.cpp
            Print.cpp
            SPIFFS.cpp
            stdlib_noniso.c
            Stream.cpp
            WString.cpp)

target_include_directories(system_mocks PUBLIC
        ${PROJECT_SOURCE_DIR}/test/system_mocks)
