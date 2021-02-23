const app = Vue.createApp({
    data() {
        return {
            update_count: 0,
            state_change_count: 0
        }
    },
    methods: {
        updateUI()
        {
            this.update_count++;
        },
        stateChanged() {
            this.state_change_count++;
        }
    }
})