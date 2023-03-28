<template>
  <div class="container">
    <va-card tag="div">
      <va-card-title>Регистрация</va-card-title>
      <va-card-content>
        <va-form tag="form" ref="form" style="width: 250px;" @submit.prevent="onSubmit" @validation="validation = $event">
          <p class="error" v-if="error">{{error}}</p>
          <va-input
              v-model="email"
              label="Email"
              type="email"
              :rules="[(value) => (value && value.length > 0) || 'Введите email',  (value) => String(value).toLowerCase().match(/^[\w-.]+@([\w-]+\.)+[a-z]{2,5}$/) || 'Не валидный email']"
          />
          <va-input
              v-model="login"
              class="mt-3"
              label="Логин"
              :rules="[(value) => (value && value.length > 0) || 'Введите логин',]"
          />
          <va-input
              v-model="password"
              class="mt-3"
              label="Пароль"
              type="password"
              :rules="[(value) => (value && value.length > 8) || 'Слишком короткий пароль']"
          />
          <div class="container">
            <va-button
                type="submit"
                class="mt-3"
            >
              Регистрация
            </va-button>

            <router-link to="/login" class="link-small">Войти</router-link>
          </div>

        </va-form>
      </va-card-content>
    </va-card>
  </div>
</template>

<script>
import {registerUser} from "@/api/auth";

export default {
  name: "RegisterView",
  data(){
    return {
      login: "",
      password: "",
      email: "",
      error: "",
      validation: null,
    }
  },
  methods: {
    onSubmit(){
      if (!this.validation) {
        this.validation = this.$refs.form.validate();
        if (!this.validation)
          return;
      }
      registerUser(this.login, this.password, this.email).then((userdata) => {
        this.$emit('userSet', userdata);
        localStorage.setItem('userName', userdata.username)
        this.$router.push('/login')
      }).catch((err) => this.error = String(err))
    },

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