import {createRouter, createWebHashHistory} from "vue-router";
import HomeView from "@/views/HomeView.vue";
import RegisterView from "@/views/RegisterView.vue";
import LoginView from "@/views/LoginView.vue";
import JokeFormView from "@/views/JokeFormView.vue";
import MyJokesView from "@/views/MyJokesView.vue";
import JokeView from "@/views/JokeView.vue";
import ProfileView from "@/views/ProfileView.vue";

const routes = [
    { path: '/', component: HomeView },
    { path: '/register', component: RegisterView },
    { path: '/profile', component: ProfileView },
    { path: '/login', component: LoginView },
    { path: '/joke/new', component: JokeFormView },
    { path: '/joke/my', component: MyJokesView },
    { path: '/jokes', component: JokeView    },
]

const router = createRouter({
    // 4. Provide the history implementation to use. We are using the hash history for simplicity here.
    history: createWebHashHistory(),
    routes, // short for `routes: routes`
})

export default router;