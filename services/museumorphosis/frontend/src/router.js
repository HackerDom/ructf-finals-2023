import {createWebHashHistory, createRouter} from 'vue-router';
import HomeView from "@/views/HomeView.vue";
import LoginView from "@/views/LoginView.vue";
import MuseumListView from "@/views/MuseumListView.vue";
import MuseumView from "@/views/MuseumView.vue";
import RegisterView from "@/views/RegisterView.vue";
import SaveExhibitView from "@/views/SaveExhibit.vue";

const routes = [
    { path: '/', component: HomeView },
    { path: '/login', component: LoginView },
    { path: '/register', component: RegisterView },
    { path: '/museums', component: MuseumListView},
    { path: '/museums/:id', component: MuseumView, props: true, name: "Museum"},
    { path: '/exhibit/create', component: SaveExhibitView },
    // { path: '/museum/exhibits', component: ExhibitView },
]

const router = createRouter({
    // 4. Provide the history implementation to use. We are using the hash history for simplicity here.
    history: createWebHashHistory(),
    routes, // short for `routes: routes`
})

export default router;