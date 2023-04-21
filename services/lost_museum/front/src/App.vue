<template>
    <va-navbar
            color="primary"

            class="mb-2"
    >
        <template #left>
            <router-link to="/" class="link">
                <va-navbar-item class="logo">
                    АНЕКДОТ.РУ
                </va-navbar-item>
            </router-link>
        </template>

        <template #right>
            <va-navbar-item><router-link v-if="token !== null" to="/joke/my" class="link">Мои анекдоты</router-link></va-navbar-item>
            <va-navbar-item><router-link to="/jokes" class="link">Все анекдоты</router-link></va-navbar-item>
            <va-navbar-item><router-link v-if="token !== null" to="/joke/new" class="link">Новый анекдот</router-link></va-navbar-item>
            <va-navbar-item v-if="token === null"><router-link to="/login" class="link">Sign In</router-link></va-navbar-item>
            <va-navbar-item v-if="token === null"><router-link to="/register" class="link">Register</router-link></va-navbar-item>

            <va-navbar-item><router-link v-if="token !== null" to="/profile" class="link">Мои друзья</router-link></va-navbar-item>
            <va-navbar-item v-if="token !== null" @click="onLogout" class="link">Logout</va-navbar-item>
        </template>
    </va-navbar>
    <router-view :token="token" @login="onLogin" @userSet="onUserSet">

    </router-view>
</template>

<script>
export default {
    name: 'App',
    data(){
        return {
            token: localStorage.getItem('token'),
            userName: localStorage.getItem('userName')
        }
    },
    mounted() {
    },
    methods: {
        onLogin(token){
            this.token = token;
        },
        onUserSet(userdata){
            this.userName = userdata.username;
        },
        onLogout(){
            this.token = null;
            this.userName = null;
            localStorage.removeItem('token')
            localStorage.removeItem('userName')
        }
    }
}
</script>

<style>
.upper{
    position: relative;
    z-index: 5;
}
.link{
    color: white;
    cursor: pointer;
}
.logo {
    font-weight: 600;
    font-size: 1.5rem;
}
</style>
