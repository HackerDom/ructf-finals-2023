<template>
  <div class="container">
    <va-card tag="div">
      <va-card-title>Registration</va-card-title>
      <va-card-content>
        <va-form tag="form" ref="form" style="width: 250px;" @submit.prevent="onSubmit" @validation="validation = $event">
          <p class="error" v-if="error">{{error}}</p>
          <va-input
              v-model="museum_name"
              class="mt-3"
              label="Museum Name"
              :rules="[(value) => (value && value.length > 0) || 'Enter your museum name',]"
          />
          <va-input
              v-model="password"
              class="mt-3"
              label="Password"
              type="password"
              :rules="[(value) => (value && value.length > 8) || 'Password too short']"
          />
          <div class="container">
            <va-button
                type="submit"
                class="mt-3"
            >
              Register
            </va-button>

            <router-link to="/login" class="link-small">Sign In</router-link>
          </div>

        </va-form>
      </va-card-content>
    </va-card>
  </div>
</template>

<script>
import {registerUser} from "@/api/api";

export default {
  name: "RegisterView",
  data(){
    return {
      museum_name: "",
      password: "",
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
      registerUser(this.museum_name, this.password).then((token) => {
          localStorage.setItem('token', token)
          this.$emit('login', token)
          this.$router.push('/')
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