add_executable(AlarmPolicy_uinttest
                AlarmPolicy_uinttest.cpp
                ${PROJECT_SOURCE_DIR}/src/AlarmPolicy.cpp
                ${PROJECT_SOURCE_DIR}/test/mocks/Arduino.cpp
                ${PROJECT_SOURCE_DIR}/test/mocks/ActivityLog.cpp
                ${PROJECT_SOURCE_DIR}/test/mocks/WString.cpp
                ${PROJECT_SOURCE_DIR}/test/mocks/stdlib_noniso.c)

target_link_libraries(AlarmPolicy_uinttest
                        test_main)

target_include_directories(AlarmPolicy_uinttest PUBLIC
                            ${PROJECT_SOURCE_DIR}/src
                            ${PROJECT_SOURCE_DIR}/include
                            ${PROJECT_SOURCE_DIR}/test/mocks
                            ${PROJECT_SOURCE_DIR}/lib/WavFilePlayer
                            ${PROJECT_SOURCE_DIR}/lib/Logging)