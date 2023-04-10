import {createWebHashHistory, createRouter} from 'vue-router';
import HomeView from "@/views/HomeView.vue";
import LoginView from "@/views/LoginView.vue";
import BooksList from "@/views/BooksList.vue";
import BookView from "@/views/BookView.vue";
import RegisterView from "@/views/RegisterView.vue";
import SaveBookView from "@/views/SaveBookView.vue";

const routes = [
    { path: '/', component: HomeView },
    { path: '/login', component: LoginView },
    { path: '/register', component: RegisterView },
    { path: '/books', component: BooksList },
    { path: '/book/save', component: SaveBookView },
    { path: '/books/:id', component: BookView , props: true, name: 'Book'},
]

const router = createRouter({
    // 4. Provide the history implementation to use. We are using the hash history for simplicity here.
    history: createWebHashHistory(),
    routes, // short for `routes: routes`
})

export default router;