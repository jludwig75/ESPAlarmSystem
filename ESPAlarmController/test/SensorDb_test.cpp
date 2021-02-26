#include <catch.hpp>

#include "SensorDb.h"

#include <SPIFFS.h>


SCENARIO( "Test SensorDb", "" )
{
    REQUIRE(SPIFFS.format());
    SensorDataBase db;

    WHEN( "the database is initialixed on an empty file system" )
    {
        REQUIRE(db.begin());

        THEN( "There are no sensors in the databse" )
        {
            SensorList sensorList;
            REQUIRE(db.getAlarmSensors(sensorList));
            REQUIRE(sensorList.empty());
        }

        WHEN( "a sensor is added to the database" )
        {
            AlarmSensor sensor(1, true, "Front Door", SensorState::Unknown);

            REQUIRE(db.storeSensor(sensor));

            THEN( "the sensor can retreived from the databse" )
            {
                SensorList sensorList;
                REQUIRE(db.getAlarmSensors(sensorList));
                REQUIRE(sensorList.size() == 1);
                REQUIRE(sensorList[0].id == 1);
            }

            WHEN( "a second sensor is added to the database" )
            {
                AlarmSensor sensor(1234, true, "Front Door", SensorState::Unknown);

                REQUIRE(db.storeSensor(sensor));

                THEN( "the sensor and the previous sensor can retreived from the databse" )
                {
                    SensorList sensorList;
                    REQUIRE(db.getAlarmSensors(sensorList));
                    REQUIRE(sensorList.size() == 2);
                    REQUIRE(sensorList[0].id == 1);
                    REQUIRE(sensorList[1].id == 1234);
                }

                WHEN( "a is updated" )
                {
                    const char* testSensorName = "New Sensor Name";
                    SensorList sensorList;
                    REQUIRE(db.getAlarmSensors(sensorList));
                    REQUIRE(sensorList.size() == 2);
                    REQUIRE(sensorList[1].id == 1234);
                    sensorList[1].name = testSensorName;
                    sensorList[0].enabled = false;

                    REQUIRE(db.updateSensor(sensorList[1]));
                    REQUIRE(db.updateSensor(sensorList[0]));

                    THEN( "the sensors can still be retreived and the sensor was actually updated" )
                    {
                        SensorList sensorList;
                        REQUIRE(db.getAlarmSensors(sensorList));
                        REQUIRE(sensorList.size() == 2);
                        REQUIRE(sensorList[0].id == 1);
                        REQUIRE_FALSE(sensorList[0].enabled);
                        REQUIRE(sensorList[1].id == 1234);
                        REQUIRE(sensorList[1].name == testSensorName);
                    }

                    WHEN( "the database is reloaded" )
                    {
                        db = SensorDataBase();
                        REQUIRE(db.begin());
                        THEN( "the sensors retain thier state" )
                        {
                            SensorList sensorList;
                            REQUIRE(db.getAlarmSensors(sensorList));
                            REQUIRE(sensorList.size() == 2);

                            REQUIRE(sensorList[0].id == 1);
                            REQUIRE_FALSE(sensorList[0].enabled);
                            REQUIRE(sensorList[0].name == "Front Door");

                            REQUIRE(sensorList[1].id == 1234);
                            REQUIRE(sensorList[1].enabled);
                            REQUIRE(sensorList[1].name == testSensorName);
                        }
                    }
                }
            }
        }
    }
}