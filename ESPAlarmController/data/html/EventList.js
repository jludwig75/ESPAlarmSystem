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
                    {{ event.dateTime }}
                </td>
                <td class="event-entry">
                    {{ event.message }}
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
            var eventList = events.split('\n');
            for (eventString of eventList)
            {
                if (eventString.length == 0)
                {
                    continue;
                }
                parts = eventString.split(': ')
                var timestamp = parseInt(parts[0]);
                var dtString = this.getDateTimeString(timestamp);
                this.addEvent( {
                    'timestamp': timestamp,
                    'dateTime': dtString,
                    'message': parts.slice(1).join(': ')
                });
            }
        },
        addEvent(newEvent) {
            for (evt of this.events) {
                if (newEvent.timestamp == evt.timestamp &&
                    newEvent.message == evt.message)
                {
                    return;   
                }
            }

            this.events.push(newEvent);
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