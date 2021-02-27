#include <catch.hpp>


#include "AlarmSystem.h"

#include <SPIFFS.h>

#include "protocol.h"
#include "TestESPNowServer.h"
#include "TestWavFilePlayer.h"

#include <memory>


const uint8_t sensor1MacAddress[6] = { 0x30, 0xAE, 0xA4, 0x05, 0xCE, 0x1C };
const uint64_t sensor1Id = 0x30AEA405CE1C;

const uint8_t sensor2MacAddress[6] = { 0x30, 0xAE, 0xA4, 0x05, 0xCE, 0xAB };
const uint64_t sensor2Id = 0x30AEA405CEAB;


SCENARIO( "Test AlarmSystem", "[]" )
{
    GIVEN ( "an alarm system" )
    {
        REQUIRE(SPIFFS.format());
        auto alarm = std::make_unique<AlarmSystem>("", "", 0, 0, 0);
        alarm->begin();

        WHEN( "no sensors have been reported" )
        {
            REQUIRE(alarm->sensors().empty());
            REQUIRE_FALSE(alarm->canArm());
            REQUIRE(alarm->validOperations().empty());
            REQUIRE_FALSE(alarm->arm());

            THEN( "we cannot retreive a sensor" )
            {
                REQUIRE(alarm->getSensor(sensor1Id) == nullptr);
            }

            THEN( "cannot update a non-existent sensor" )
            {
                AlarmSensor sensor(sensor1Id, true, "Front Door", SensorState::Unknown);
                REQUIRE_FALSE(alarm->updateSensor(sensor));
            }

            WHEN( "a sensor is reported" )
            {
                SensorState state{ESP_SLEEP_WAKEUP_UNDEFINED, SensorState::State::Closed, 3.3};
                TestESPNowServer::instance().send(sensor1MacAddress, reinterpret_cast<const uint8_t*>(&state), sizeof(state));

                // Run loop so message gets delivered.
                alarm->onLoop();

                THEN( "an unenabled sensor is reported" )
                {
                    auto sensors = alarm->sensors();
                    REQUIRE(sensors.size() == 1);
                    REQUIRE_FALSE(alarm->canArm());
                    REQUIRE(alarm->validOperations().empty());
                    REQUIRE_FALSE(alarm->arm());

                    const auto& sensor = sensors[sensor1Id];
                    REQUIRE(sensor.id == sensor1Id);
                    REQUIRE(sensor.name == "");
                    REQUIRE_FALSE(sensor.enabled);

                }

                THEN( "we can retreive the sensor" )
                {
                    REQUIRE(alarm->getSensor(sensor1Id) != nullptr);
                    REQUIRE(alarm->getSensor(sensor1Id)->id == sensor1Id);
                }

                THEN( "we can retreive a const verion of the sensor from a const instance of the class" )
                {
                    const AlarmSystem& alarmSystem = *alarm.get();
                    REQUIRE(alarmSystem.getSensor(sensor1Id) != nullptr);
                    REQUIRE(alarmSystem.getSensor(sensor1Id)->id == sensor1Id);
                }

                THEN( "we cannot retreive a non-existent sensor" )
                {
                    REQUIRE(alarm->getSensor(56754) == nullptr);
                }

                THEN( "cannot update a non-existent sensor" )
                {
                    AlarmSensor sensor(654654, true, "Front Door", SensorState::Unknown);
                    REQUIRE_FALSE(alarm->updateSensor(sensor));
                }

                WHEN( "the sensor is named" )
                {
                    const char* sensor1Name = "Back Door";

                    auto sensors = alarm->sensors();
                    auto sensor = sensors[sensor1Id];
                    sensor.name = sensor1Name;

                    REQUIRE(alarm->updateSensor(sensor));
                    // re-get the sensor to make sure it was updated in AlarmSystem
                    sensors = alarm->sensors();
                    sensor = sensors[sensor1Id];

                    THEN( "we can retreive the name" )
                    {
                        REQUIRE(sensor.id == sensor1Id);
                        REQUIRE(sensor.name == sensor1Name);
                        REQUIRE_FALSE(sensor.enabled);
                    }

                    WHEN( "the sensor is enabled" )
                    {
                        sensor.enabled = true;

                        REQUIRE(alarm->updateSensor(sensor));
                        // re-get the sensor to make sure it was updated in AlarmSystem
                        sensors = alarm->sensors();
                        sensor = sensors[sensor1Id];

                        THEN( "the system can be armed" )
                        {
                            REQUIRE(sensor.id == sensor1Id);
                            REQUIRE(sensor.enabled);

                            REQUIRE(alarm->canArm());
                            REQUIRE(alarm->validOperations() == std::vector<AlarmOperation>{ {AlarmOperation::Arm} });
                        }

                        THEN( "the system says it's disarmed" )
                        {
                            REQUIRE(alarm->state() == AlarmState::Disarmed);
                        }

                        THEN( "disarming does nothing" )
                        {
                            alarm->disarm();
                            REQUIRE(alarm->state() == AlarmState::Disarmed);
                        }

                        WHEN( "a sensor is opened" )
                        {
                            // Clear out all sounds
                            while (numberOfAudioFilesPlayed() > 0)
                            {
                                lastAudioFilePlayed();
                            }
                            SensorState state{ESP_SLEEP_WAKEUP_UNDEFINED, SensorState::State::Open, 3.3};
                            TestESPNowServer::instance().send(sensor1MacAddress, reinterpret_cast<const uint8_t*>(&state), sizeof(state));
                            alarm->onLoop();

                            THEN( "an open chime is played" )
                            {
                                REQUIRE(numberOfAudioFilesPlayed() == 1);
                                REQUIRE(lastAudioFilePlayed() == "/C_OPEN.WAV");

                                WHEN( "a sensor is closed" )
                                {
                                    SensorState state{ESP_SLEEP_WAKEUP_UNDEFINED, SensorState::State::Closed, 3.3};
                                    TestESPNowServer::instance().send(sensor1MacAddress, reinterpret_cast<const uint8_t*>(&state), sizeof(state));
                                    alarm->onLoop();

                                    THEN( "a close chime is played" )
                                    {
                                        REQUIRE(numberOfAudioFilesPlayed() == 1);
                                        // TODO: set sounds: REQUIRE(lastAudioFilePlayed() == "/C_CLOSE.WAV");
                                        REQUIRE(lastAudioFilePlayed() == "/C_OPEN.WAV");
                                    }
                                }
                            }
                        }

                        WHEN( "the alram is armed" )
                        {
                            REQUIRE(alarm->arm());

                            THEN( "the system says it's armed" )
                            {
                                REQUIRE(alarm->state() == AlarmState::Armed);
                            }

                            THEN( "it can be disarmed" )
                            {
                                REQUIRE(alarm->validOperations() == std::vector<AlarmOperation>{ {AlarmOperation::Disarm} });
                            }

                            THEN( "re-arming does nothing" )
                            {
                                REQUIRE(alarm->arm());
                                REQUIRE(alarm->state() == AlarmState::Armed);
                            }

                            THEN( "sensors cannot be modified" )
                            {
                                REQUIRE_FALSE(alarm->updateSensor(sensor));
                            }

                            WHEN( "the system is disarmed" )
                            {
                                alarm->disarm();
                                THEN( "it can be re-armed" )
                                {
                                    REQUIRE(alarm->canArm());
                                    REQUIRE(alarm->validOperations() == std::vector<AlarmOperation>{ {AlarmOperation::Arm} });
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    GIVEN ( "an alarm system with two closed sensors" )
    {
        REQUIRE(SPIFFS.format());
        while (numberOfAudioFilesPlayed() > 0)
        {
            lastAudioFilePlayed();
        }
        auto alarm = std::make_unique<AlarmSystem>("", "", 0, 0, 0);
        alarm->begin();

        // Report the sensors as closed
        SensorState state{ESP_SLEEP_WAKEUP_UNDEFINED, SensorState::State::Closed, 3.3};
        TestESPNowServer::instance().send(sensor1MacAddress, reinterpret_cast<const uint8_t*>(&state), sizeof(state));

        TestESPNowServer::instance().send(sensor2MacAddress, reinterpret_cast<const uint8_t*>(&state), sizeof(state));

        // Advance the loop to get the sensor updates
        // Once for each sensor
        alarm->onLoop();
        alarm->onLoop();

        REQUIRE(alarm->sensors().size() == 2);

        // Enable both sensors
        for (auto pair : alarm->sensors())
        {
            auto sensor = pair.second;
            sensor.enabled = true;
            REQUIRE(alarm->updateSensor(sensor));
        }

        REQUIRE(numberOfAudioFilesPlayed() == 0);

        // arm the system
        REQUIRE(alarm->arm());
        REQUIRE(alarm->state() == AlarmState::Armed);

        // Arming sound must have been played
        REQUIRE(numberOfAudioFilesPlayed() == 1);
        // TODO: change sound files: REQUIRE(lastAudioFilePlayed() == "/A_ARM.WAV");
        REQUIRE(lastAudioFilePlayed() == "/C_OPEN.WAV");

        WHEN( "The alarm system is reset" )
        {
            alarm.reset();
            while (numberOfAudioFilesPlayed() > 0)
            {
                lastAudioFilePlayed();
            }
            alarm = std::make_unique<AlarmSystem>("", "", 0, 0, 0);
            alarm->begin();

            REQUIRE(numberOfAudioFilesPlayed() == 0);

            THEN( "the alarm is still armed" )
            {
                REQUIRE(alarm->state() == AlarmState::Armed);
            }
        }

        WHEN( "a sensor is opened" )
        {
            state.state = SensorState::State::Open;
            TestESPNowServer::instance().send(sensor2MacAddress, reinterpret_cast<const uint8_t*>(&state), sizeof(state));

            // Advance loop to report sensor
            alarm->onLoop();

            THEN( "the alarm sound is played" )
            {
                REQUIRE(numberOfAudioFilesPlayed() == 1);
                REQUIRE(lastAudioFilePlayed() == "/A_SOUND.WAV");
            }

            THEN( "the alram is triggered ")
            {
                REQUIRE(alarm->state() == AlarmState::AlarmTriggered);
                alarm->onLoop();
            }

            WHEN( "The alarm system is reset" )
            {
                alarm.reset();
                while (numberOfAudioFilesPlayed() > 0)
                {
                    lastAudioFilePlayed();
                }
                alarm = std::make_unique<AlarmSystem>("", "", 0, 0, 0);
                alarm->begin();
                // Advance loop once to play alarm triggered sound
                alarm->onLoop();

                THEN( "the alarm is still triggered" )
                {
                    REQUIRE(alarm->state() == AlarmState::AlarmTriggered);
                }
                    
                THEN( "the alarm sound is played again" )
                {
                    REQUIRE(numberOfAudioFilesPlayed() == 1);
                    REQUIRE(lastAudioFilePlayed() == "/A_SOUND.WAV");

                    WHEN( "there is a long delay" )
                    {
                        delay(1500);
                        alarm->onLoop();

                        THEN( "the alarm is still triggered" )
                        {
                            REQUIRE(alarm->state() == AlarmState::AlarmTriggered);
                        }
                        
                        THEN( "the alarm sound is played again" )
                        {
                            REQUIRE(numberOfAudioFilesPlayed() == 1);
                            REQUIRE(lastAudioFilePlayed() == "/A_SOUND.WAV");
                        }
                    }
                }
            }
        }
   }
}