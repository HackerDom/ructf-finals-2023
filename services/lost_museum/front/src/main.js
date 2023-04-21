import { createApp } from 'vue'
import App from './App.vue'
import router from "@/router";
import { createVuestic } from "vuestic-ui";
import "vuestic-ui/css";

const app = createApp(App);
app.use(router);
app.use(createVuestic({
    config: {
        colors: {
            variables: {
                primary: "#B456C0",
                custom: "#A35600",
            },
        },
    },
}));
app.mount('#app')
