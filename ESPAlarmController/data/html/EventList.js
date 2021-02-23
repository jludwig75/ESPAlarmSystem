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
            eventList = this.parseEventList(events);
            for (let evt of eventList) {
                if (!this.events.find(evt2 => evt2.id == evt.id)) {
                    this.events.push(evt);
                }
            }
            for (let i = 0; i < this.events.length; ) {
                if (!eventList.find(evt2 => evt2.id == this.events[i].id)) {
                    this.events.splice(i, 1);
                }
                else
                {
                    i++;
                }
            }
        },
        parseEventList(events) {
            var eventList = events.split('\n');
            var parsedEvents = [];
            for (let eventString of eventList)
            {
                if (eventString.length == 0)
                {
                    continue;
                }
                parts = eventString.split(':|:')
                if (parts.length != 3)
                {
                    continue;
                }
                var id = parseInt(parts[0]);
                var dtString = this.getDateTimeString(parseInt(parts[1]));
                parsedEvents.push({
                    'id': id,
                    'dateTime': dtString,
                    'message': parts[2]
                });
            }

            return parsedEvents;
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