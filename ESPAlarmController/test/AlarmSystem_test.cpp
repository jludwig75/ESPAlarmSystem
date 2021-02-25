#include <catch.hpp>


#include "AlarmSystem.h"

#include <SPIFFS.h>

#include "protocol.h"
#include "TestESPNowServer.h"


const uint8_t sensor1MacAddress[6] = { 0x30, 0xAE, 0xA4, 0x05, 0xCE, 0x1C };
const uint64_t sensor1Id = 0x30AEA405CE1C;


SCENARIO( "Test AlarmSystem", "[]" )
{
    GIVEN ( "an alarm system" )
    {
        REQUIRE(SPIFFS.format());
        AlarmSystem alarm("", "", 0, 0, 0);
        alarm.begin();

        WHEN( "no sensors have been reported" )
        {
            REQUIRE(alarm.sensors().empty());
            REQUIRE_FALSE(alarm.canArm());
            REQUIRE(alarm.validOperations().empty());
            REQUIRE_FALSE(alarm.arm());

            THEN( "we cannot retreive a sensor" )
            {
                REQUIRE(alarm.getSensor(sensor1Id) == nullptr);
            }

            THEN( "cannot update a non-existent sensor" )
            {
                AlarmSensor sensor(sensor1Id, true, "Front Door", SensorState::Unknown);
                REQUIRE_FALSE(alarm.updateSensor(sensor));
            }

            WHEN( "a sensor is reported" )
            {
                SensorState state{ESP_SLEEP_WAKEUP_UNDEFINED, SensorState::State::Closed, 3.3};
                TestESPNowServer::instance().send(sensor1MacAddress, reinterpret_cast<const uint8_t*>(&state), sizeof(state));

                // Run loop so message gets delivered.
                alarm.onLoop();

                THEN( "an unenabled sensor is reported" )
                {
                    auto sensors = alarm.sensors();
                    REQUIRE(sensors.size() == 1);
                    REQUIRE_FALSE(alarm.canArm());
                    REQUIRE(alarm.validOperations().empty());
                    REQUIRE_FALSE(alarm.arm());

                    const auto& sensor = sensors[sensor1Id];
                    REQUIRE(sensor.id == sensor1Id);
                    REQUIRE(sensor.name == "");
                    REQUIRE_FALSE(sensor.enabled);

                }

                THEN( "we can retreive the sensor" )
                {
                    REQUIRE(alarm.getSensor(sensor1Id) != nullptr);
                    REQUIRE(alarm.getSensor(sensor1Id)->id == sensor1Id);
                }

                THEN( "we can retreive a const verion of the sensor from a const instance of the class" )
                {
                    const AlarmSystem& alarmSystem = alarm;
                    REQUIRE(alarmSystem.getSensor(sensor1Id) != nullptr);
                    REQUIRE(alarmSystem.getSensor(sensor1Id)->id == sensor1Id);
                }

                THEN( "we cannot retreive a non-existent sensor" )
                {
                    REQUIRE(alarm.getSensor(56754) == nullptr);
                }

                THEN( "cannot update a non-existent sensor" )
                {
                    AlarmSensor sensor(654654, true, "Front Door", SensorState::Unknown);
                    REQUIRE_FALSE(alarm.updateSensor(sensor));
                }

                WHEN( "the sensor is named" )
                {
                    const char* sensor1Name = "Back Door";

                    auto sensors = alarm.sensors();
                    auto sensor = sensors[sensor1Id];
                    sensor.name = sensor1Name;

                    REQUIRE(alarm.updateSensor(sensor));
                    // re-get the sensor to make sure it was updated in AlarmSystem
                    sensors = alarm.sensors();
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

                        REQUIRE(alarm.updateSensor(sensor));
                        // re-get the sensor to make sure it was updated in AlarmSystem
                        sensors = alarm.sensors();
                        sensor = sensors[sensor1Id];

                        THEN( "the system can be armed" )
                        {
                            REQUIRE(sensor.id == sensor1Id);
                            REQUIRE(sensor.enabled);

                            REQUIRE(alarm.canArm());
                            REQUIRE(alarm.validOperations() == std::vector<AlarmOperation>{ {AlarmOperation::Arm} });
                        }

                        THEN( "the system says it's disarmed" )
                        {
                            REQUIRE(alarm.state() == AlarmState::Disarmed);
                        }

                        THEN( "disarming does nothing" )
                        {
                            alarm.disarm();
                            REQUIRE(alarm.state() == AlarmState::Disarmed);
                        }

                        WHEN( "the alram is armed" )
                        {
                            REQUIRE(alarm.arm());

                            THEN( "the system says it's armed" )
                            {
                                REQUIRE(alarm.state() == AlarmState::Armed);
                            }

                            THEN( "it can be disarmed" )
                            {
                                REQUIRE(alarm.validOperations() == std::vector<AlarmOperation>{ {AlarmOperation::Disarm} });
                            }

                            THEN( "re-arming does nothing" )
                            {
                                REQUIRE(alarm.arm());
                                REQUIRE(alarm.state() == AlarmState::Armed);
                            }

                            THEN( "sensors cannot be modified" )
                            {
                                REQUIRE_FALSE(alarm.updateSensor(sensor));
                            }

                            WHEN( "the system is disarmed" )
                            {
                                alarm.disarm();
                                THEN( "it can be re-armed" )
                                {
                                    REQUIRE(alarm.canArm());
                                    REQUIRE(alarm.validOperations() == std::vector<AlarmOperation>{ {AlarmOperation::Arm} });
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}