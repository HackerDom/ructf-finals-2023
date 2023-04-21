<template>
  <va-navbar
      color="primary"
      class="mb-2"
  >
    <template #left>
      <router-link to="/" class="link">
        <va-navbar-item class="logo">
            Museumorphosis
        </va-navbar-item>
      </router-link>
    </template>

    <template #right>
<!--      <va-navbar-item><router-link v-if="token !== null" to="/books" class="link">My books</router-link></va-navbar-item>-->
      <va-navbar-item><router-link  to="/museums" class="link">Look other museums</router-link></va-navbar-item>
        <va-navbar-item><router-link  v-if="token !== null" to="/exhibit/create" class="link">Create Exhibit</router-link></va-navbar-item>
      <va-navbar-item v-if="token === null"><router-link to="/login" class="link">Sign In</router-link></va-navbar-item>
      <va-navbar-item v-if="token === null"><router-link to="/register" class="link">Register</router-link></va-navbar-item>
      <va-navbar-item v-if="token !== null" @click="onLogout" class="link">Logout</va-navbar-item>
    </template>
  </va-navbar>
  <router-view :token="token" @login="onLogin">

  </router-view>
</template>

<script>


export default {
  name: 'App',
  data(){
    return {
      token: localStorage.getItem('token'),
    }
  },
  mounted() {

  },
  methods: {
    onLogin(token){
      this.token = token;
    },
    onLogout(){
        localStorage.removeItem('token');
        this.token = null;
        this.$router.push('/');
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
