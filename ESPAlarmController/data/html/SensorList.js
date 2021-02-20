app.component('sensor-list-view', {
    template:
    /*html*/
    `
<div class="display-group">
    <fieldset>
        <legend>Alarm Sensors</legend>
        <table>
            <tr>
                <th>ID</th>
                <th>Status</th>
                <th>Enabled</th>
                <th>Name</th>
            </tr>
            <tr v-for="sensor in sensors">
                <td>
                    {{ sensor.id }}
                </td>
                <td>
                    {{ sensor.state }}
                </td>
                <td>
                    {{ sensor.enabled }}
                    <button v-on:click="toggleSensorEnanbled(sensor)">
                        {{ sensor.enabled == "yes" ? "Disable" : "Enable" }}
                    </button>
                </td>
                <td>
                    <span v-if="sensor.edit">
                        <input type="text" id="sensorName" v-model="sensor.newName">
                        <button v-on:click="updateName(sensor)">Update</button>
                        <button v-on:click="cancelUpdateName(sensor)">Cancel</button>
                    </span>
                    <span v-else>
                        {{ sensor.name }}
                        <button v-on:click="editName(sensor)">Edit</button>
                    </span>
                </td>
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
                    if (!("newName" in sensor))
                    {
                        sensor["newName"] = "";
                        sensor["edit"] = false;
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
        },
        toggleSensorEnanbled(sensor)
        {
            axios.
                put('/alarm_system/sensor/' + sensor.id + "?enabled=" + this.toggleEnabledState(sensor.enabled)).    // Toggle the current state
                then(response => this.toggleSensorEnanbledSuccess(sensor)).
                catch(error => console.log("Failed to toggle sensor " + sensor.id + " state"));
        },
        toggleSensorEnanbledSuccess(sensor)
        {
            sensor.enabled = this.toggleEnabledState(sensor.enabled);    // Toggle the current state
        },
        toggleEnabledState(state)
        {
            if (state == "yes")
            {
                return "no";
            }

            return "yes";
        },
        editName(sensor)
        {
            sensor.edit = true;
            sensor.newName = sensor.name;
        },
        updateName(sensor)
        {
            sensor.edit = false;
            var putUrl = '/alarm_system/sensor/' + sensor.id + "?name=" + encodeURIComponent(sensor.newName);
            axios.
                put(putUrl).
                then(response => this.updateNameResponse(sensor, true, response)).
                catch(error => this.updateNameResponse(sensor, false, error));
        },
        cancelUpdateName(sensor)
        {
            sensor.edit = false;
        },
        updateNameResponse(sensor, success, error)
        {
            if (success)
            {
                console.log("Successfully update sensor " + sensor.id + " name to \"" + sensor.newName + "\"");
            }
            else
            {
                console.log("Failed update sensor name: " + error.message);
                sensor.newName = sensor.name;
            }
            this.getSensorDetails(sensor.id);
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