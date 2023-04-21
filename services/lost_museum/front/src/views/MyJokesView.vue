<template>
    <div class="container ">

        <va-progress-circle v-if="loading" indeterminate color="primary" />
        <JokeCard v-for="joke in jokes" :key="joke.theme" :joke="joke"/>
        <p v-if="!loading && jokes.length === 0">У тебя нет анекдотов</p>
    </div>
</template>

<script>
import JokeCard from "@/components/JokeCard.vue";
import {getMyJokes} from "@/api/jokes";

export default {
    name: "MyJokesView",
    components: {JokeCard},
    props: ['token'],
    data(){
        return {
            jokes: [],
            loading: true
        }
    },
    mounted() {
        this.loading = true
        getMyJokes(this.token).then((resp) => {
            this.jokes = resp;
            this.loading = false
        }).catch((error) => this.$vaToast.init({ message: String(error), position: 'bottom-right' }))
    }
}
</script>

<style scoped>
.container {
    display: flex;
    justify-content: center;
    align-items: center;
    flex-direction: column;
    height: 100%;
    padding: 20px;
}
</style>