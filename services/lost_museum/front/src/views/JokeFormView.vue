<template>
    <div class="container">
        <va-card>
            <va-card-title>
                <h2>Новый анекдот</h2>
            </va-card-title>
            <va-card-content >
                <va-form ref="form" tag="form" @submit.prevent="onSubmit" style="max-width: 500px;">
                    <va-input
                            v-model="theme"
                            label="Тема"
                            class="mb-6"
                            style="width: 100%"
                            :rules="[(value) => (value && value.length > 0) || 'Введите тему', (value) => (value && !value.match('[0-9]') || 'Тема без цифр')]"
                    />
                    <br>

                    <va-select
                        v-model="status"
                        class="mb-6 select"
                        label="Статус"
                        style="width: 100%"
                        :options="options"
                        bordered
                    />
                    <br>
                    <va-input
                        v-model="text"
                        class="mb-6"
                        type="textarea"
                        style="width: 100%"
                        label="Текст анекдота"
                        :rules="[(value) => (value && value.length > 0) || 'Введите текст']"
                        autosize
                    />

                    <div class="container">
                        <va-button
                                type="submit"
                                class="mt-3"
                        >
                            Сохранить
                        </va-button>
                    </div>
                </va-form>
            </va-card-content>
        </va-card>
    </div>
</template>

<script>
import {saveNewJoke} from "@/api/jokes";
import router from "@/router";
export default {
    name: "SaveBookView",
    props:['token'],
    data() {
        return {
            status: "private",
            theme: "",
            text: "",
            options: [
                "private",
                "public",
            ],
        };
    },
    methods: {
        onSubmit(){
            saveNewJoke(this.token, this.theme, this.status, this.text)
                .then(() => {
                    router.push('/joke/my')
                })
                .catch((error) => {
                    this.$vaToast.init({ message: String(error), position: 'bottom-right' })
                });
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
h2{
}
.mb-6{
    margin-top: 16px;
    margin-bottom: 15px;
}
.select{
    z-index: 69;
}

</style>

<style>
.va-select-dropdown__content{
    z-index: 2;
}
</style>