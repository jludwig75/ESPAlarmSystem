app.component('event-list-view', {
    template:
    /*html*/
    `
<div id="events-list">
    <fieldset>
        <legend>Alarm Events</legend>
        <div v-for="event in events">
            {{ event }}
        </div>
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
            this.events = events.split('\n');
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