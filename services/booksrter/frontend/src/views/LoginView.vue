<template>
  <div class="container">
    <va-card tag="div">
      <va-card-title>Вход</va-card-title>
      <va-card-content>
        <va-form style="width: 250px;">
          <p class="error" v-if="error">{{error}}</p>
          <va-input
              v-model="login"
              label="Login"
              :rules="[(value) => (value && value.length > 0) || 'Enter login']"
          />
          <va-input
              v-model="password"
              class="mt-3"
              label="Password"
              type="password"
              :rules="[(value) => (value && value.length > 0) || 'Enter password']"
          />
          <div class="container">
            <va-button
                type="submit"
                class="mt-3"
                @click="onSubmit"
            >
              Sign In
            </va-button>

            <router-link to="/register" class="link-small">Registration</router-link>
          </div>

        </va-form>
      </va-card-content>
    </va-card>
  </div>
</template>

<script>
import {loginGetToken} from "@/api/auth";

export default {
  name: "LoginView",
  data() {
    return {
      login: "",
      password: "",
      error: "",
    };
  },
  methods: {
    onSubmit(){
      loginGetToken(this.login, this.password)
          .then((token) => {
            localStorage.setItem('token', token)
            this.$emit('login', token)
            this.$router.push('/')
          })
          .catch((error) => {
            console.log(error)
            this.error = error
          })
    }
  }
}
</script>

<style scoped>
.error {
  color: red;
}
.container {
  display: flex;
  justify-content: center;
  align-items: center;
  flex-direction: column;
  height: 100%;
  padding: 20px;
}
.link-small {
  font-size: 12px;
  margin-top: 10px;
}
</style>