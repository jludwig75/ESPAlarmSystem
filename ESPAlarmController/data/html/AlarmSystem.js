app.component('alarm-system-view', {
    template:
    /*html*/
    `
<div class="display-group">
    <div>
        <fieldset>
            <legend>Alarm Status</legend>
            <span id="alarm-status">{{ alarm_state }}</span>
        </fieldset>
    </div>
    <div>
        <button :disabled="!valid_operations.includes('Arm')" v-on:click="armAlarm">Arm</button>
        <button  :disabled="!valid_operations.includes('Disarm')" v-on:click="disarmAlarm">Disarm</button>
    </div>
</div>
`,
    data() {
        return {
            alarm_state: "Loading...",
            valid_operations: []
        }
    },
    methods: {
        getAlarmState() {
            axios.
                get('/alarm_system/state').
                then(response => this.gotAlarmState(response.data)).
                catch(error => console.log('Failed to get alarm system state: ' + error));
        },
        gotAlarmState(alarm_state) {
            this.alarm_state = alarm_state;
        },
        getValidOperations() {
            axios.
                get('/alarm_system/operation').
                then(response => this.gotValidOperations(response.data)).
                catch(error => console.log('Failed to get valid alarm system operations: ' + error));
        },
        gotValidOperations(valid_operations) {
            this.valid_operations = valid_operations;
        },
        refreshBackendData() {
            this.getAlarmState();
            this.getValidOperations();
        },
        armAlarm() {
            axios.
                post('/alarm_system/operation?operation=Arm').
                then(response => this.handleOperationSuccess()).
                catch(error => this.handleOperationError());
        },
        disarmAlarm() {
            axios.
                post('/alarm_system/operation?operation=Disarm').
                then(response => this.handleOperationSuccess()).
                catch(error => this.handleOperationError());
        },
        handleOperationSuccess()
        {
            this.refreshBackendData();
            // this.$emit('state-changed'); Disable for now.
        },
        handleOperationError()
        {
            this.refreshBackendData();
        }
    },
    mounted() {
        this.refreshBackendData();
        this.polling = setInterval(this.refreshBackendData, 3 * 1000);
    },
    beforeUnmount() {
        clearInterval(this.polling);
    }
});
