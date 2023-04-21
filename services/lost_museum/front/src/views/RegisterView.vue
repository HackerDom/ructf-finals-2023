


<template>
    <div class="container">
        <va-card tag="div">
            <va-card-title>Registration</va-card-title>
            <va-card-content>
                <va-form tag="form" ref="form" style="width: 250px;" @submit.prevent="onSubmit" @validation="validation = $event">
                    <p class="error" v-if="error">{{error}}</p>
                    <va-input
                            v-model="login"
                            class="mt-3"
                            label="Username"
                            :rules="[(value) => (value && value.length > 0) || 'Enter username',]"
                    />
                    <va-input
                            v-model="password"
                            class="mt-3"
                            label="Password"
                            type="password"
                            :rules="[(value) => (value && value.length > 5) || 'Password too short']"
                    />
                    <va-input
                        v-model="password_repeat"
                        class="mt-3"
                        label="Password repeat"
                        type="password"
                        :rules="[(value) => (value && value === password) || 'Password not match']"
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
import {registerUser} from "@/api/auth";
export default {
    name: "RegisterView",
    data(){
        return {
            login: "",
            password: "",
            error: "",
            password_repeat: "",
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
            registerUser(this.login, this.password).then((userdata) => {
                this.$emit('userSet', userdata);
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