app.component('event-list-view', {
    template:
    /*html*/
    `
<div id="events-list">
    <fieldset>
        <legend>Alarm Events</legend>
        <table>
            <tr v-for="event in events">
                <td class="timestamp-field event-entry">
                    {{ event[0] }}
                </td>
                <td class="event-entry">
                    {{ event[1] }}
                </td>
            </tr>
        </table>
    </fieldset>
</div>
`,
    data() {
        return {
            events: []
        }
    },
    methods: {
        getEvents() {
            axios.
                get('/alarm_system/events').
                then(response => this.gotEvents(response.data)).
                catch(error => console.log('Failed to get alarm system activity log: ' + error));
        },
        gotEvents(events) {
            this.events = [];
            var eventList = events.split('\n');
            for (eventString of eventList)
            {
                if (eventString.length == 0)
                {
                    continue;
                }
                parts = eventString.split(': ')
                console.log("parts" + parts);
                var dtString = this.getDateTimeString(parseInt(parts[0]));
                this.events.push([dtString, parts.slice(1).join(': ')]);
            }
        },
        getDateTimeString(epochTime)
        {
            var dt = new Date(0);
            dt.setUTCSeconds(epochTime);
            return dt.toLocaleString();
        }
    },
    mounted() {
        this.getEvents();
        this.polling = setInterval(this.getEvents, 5 * 1000);
    },
    beforeUnmount() {
        clearInterval(this.polling);
    }
});