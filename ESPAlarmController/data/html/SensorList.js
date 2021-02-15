app.component('sensor-list-view', {
    template:
    /*html*/
    `
    <div class="display-group">
    <fieldset>
        <legend>Alarm Sensors</legend>
        <table>
            <tr>
                <th>ID</th><th>Status</th>
            </tr>
            <tr v-for="sensor in sensors">
                <td>{{ sensor.id }}</td><td>{{ sensor.state }}</td>
            </tr>
        </table>
    </fieldset>
</div>
`,
    data() {
        return {
            sensors: []
        }
    },
    methods: {
        getSensorDetails(id) {
            axios.
                get('/alarm_system/sensor/' + id).
                then(response => this.gotSensorDetails(response.data)).
                catch(error => console.log('Failed to get alarm sensor details for sensor ' + id + ': ' + error));
        },
        gotSensorDetails(details) {
            for (sensor of this.sensors)
            {
                if (sensor.id == details['id'])
                {
                    for(k in details)
                    {
                        sensor[k]=details[k];
                    }
                    return;
                }
            }
            console.log('Sensor ' + details['id'] + ' not found in sensor list');
        },
        getSensors() {
            axios.
                get('/alarm_system/sensor').
                then(response => this.gotSensors(response.data)).
                catch(error => console.log('Failed to get alarm sensor list: ' + error));
        },
        gotSensors(sensorList) {
            for (sensorId of sensorList)
            {
                this.getSensorDetails(sensorId);
                if (!this.isKnownSensor(sensorId))
                {
                    this.sensors.push({ 'id': sensorId, 'state': 'Loading...' });
                }
            }
        },
        isKnownSensor(sensorId) {
            for (sensor of this.sensors)
            {
                if (sensor.id == sensorId)
                {
                    return true;
                }
            }

            return false;
        }
    },
    mounted() {
        this.getSensors();
        this.polling = setInterval(this.getSensors, 1000);
    },
    beforeUnmount() {
        clearInterval(this.polling);
    }
});