add_compile_definitions(ARDUINO)

add_subdirectory(system_mocks)

add_executable(AlarmPolicy_uinttest
                AlarmPolicy_uinttest.cpp
                ${PROJECT_SOURCE_DIR}/src/AlarmPolicy.cpp
                ${PROJECT_SOURCE_DIR}/test/mocks/ActivityLog.cpp)

target_link_libraries(AlarmPolicy_uinttest
                        test_main
                        system_mocks)

target_include_directories(AlarmPolicy_uinttest PUBLIC
                            ${PROJECT_SOURCE_DIR}/src
                            ${PROJECT_SOURCE_DIR}/include
                            ${PROJECT_SOURCE_DIR}/test/mocks
                            ${PROJECT_SOURCE_DIR}/lib/WavFilePlayer
                            ${PROJECT_SOURCE_DIR}/lib/Logging)

add_test(NAME AlarmPolicy_uinttest
        COMMAND AlarmPolicy_uinttest)

set_target_properties(AlarmPolicy_uinttest PROPERTIES
                        COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fPIC"
                        LINK_FLAGS "-fprofile-arcs -ftest-coverage -fPIC -lgcov")



add_executable(AlarmSystem_test
        AlarmSystem_test.cpp
        ${PROJECT_SOURCE_DIR}/src/ActivityLog.cpp
        ${PROJECT_SOURCE_DIR}/src/AlarmPersistentState.cpp
        ${PROJECT_SOURCE_DIR}/src/AlarmPolicy.cpp
        ${PROJECT_SOURCE_DIR}/src/AlarmSensor.cpp
        ${PROJECT_SOURCE_DIR}/src/AlarmSystem.cpp
        ${PROJECT_SOURCE_DIR}/src/SensorDb.cpp
        ${PROJECT_SOURCE_DIR}/src/SoundPlayer.cpp
        ${PROJECT_SOURCE_DIR}/test/mocks/AlarmWebServer.cpp
        ${PROJECT_SOURCE_DIR}/test/mocks/ESPNowServer.cpp
        ${PROJECT_SOURCE_DIR}/test/mocks/MemTracker.cpp
        ${PROJECT_SOURCE_DIR}/test/mocks/WavFilePlayer.cpp)

target_link_libraries(AlarmSystem_test
                 test_main
                 system_mocks)

target_include_directories(AlarmSystem_test PUBLIC
                    ${PROJECT_SOURCE_DIR}/src
                    ${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/test/mocks
                    ${PROJECT_SOURCE_DIR}/lib/AutoFile
                    ${PROJECT_SOURCE_DIR}/lib/ESPNowServer
                    ${PROJECT_SOURCE_DIR}/lib/Logging
                    ${PROJECT_SOURCE_DIR}/lib/MemTracker
                    ${PROJECT_SOURCE_DIR}/lib/WavFilePlayer
                    ${PROJECT_SOURCE_DIR}/.pio/libdeps/lolin32/ArduinoJson/src)

add_test(NAME AlarmSystem_test
        COMMAND AlarmSystem_test)

set_target_properties(AlarmSystem_test PROPERTIES
                        COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fPIC"
                        LINK_FLAGS "-fprofile-arcs -ftest-coverage -fPIC -lgcov")


                        
add_executable(AlarmPersistentState_unittest
        AlarmPersistentState_unittest.cpp
        ${PROJECT_SOURCE_DIR}/src/AlarmPersistentState.cpp)

target_link_libraries(AlarmPersistentState_unittest
                 test_main
                 system_mocks)

target_include_directories(AlarmPersistentState_unittest PUBLIC
                    ${PROJECT_SOURCE_DIR}/src
                    ${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/lib/AutoFile
                    ${PROJECT_SOURCE_DIR}/lib/Logging)

add_test(NAME AlarmPersistentState_unittest
        COMMAND AlarmPersistentState_unittest)

set_target_properties(AlarmPersistentState_unittest PROPERTIES
                        COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fPIC"
                        LINK_FLAGS "-fprofile-arcs -ftest-coverage -fPIC -lgcov")



                        
add_executable(SensorDb_unittest
        SensorDb_unittest.cpp
        ${PROJECT_SOURCE_DIR}/src/AlarmSensor.cpp
        ${PROJECT_SOURCE_DIR}/src/SensorDb.cpp)

target_link_libraries(SensorDb_unittest
                 test_main
                 system_mocks)

target_include_directories(SensorDb_unittest PUBLIC
                    ${PROJECT_SOURCE_DIR}/src
                    ${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/lib/AutoFile
                    ${PROJECT_SOURCE_DIR}/lib/ESPNowServer
                    ${PROJECT_SOURCE_DIR}/lib/Logging
                    ${PROJECT_SOURCE_DIR}/lib/MemTracker
                    ${PROJECT_SOURCE_DIR}/lib/WavFilePlayer
                    ${PROJECT_SOURCE_DIR}/.pio/libdeps/lolin32/ArduinoJson/src)

add_test(NAME SensorDb_unittest
        COMMAND SensorDb_unittest)

set_target_properties(SensorDb_unittest PROPERTIES
                        COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fPIC"
                        LINK_FLAGS "-fprofile-arcs -ftest-coverage -fPIC -lgcov")

