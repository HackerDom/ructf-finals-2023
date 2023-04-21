<template>
  <div class="container">
      <div class="searching">
          <va-input
                  v-model="search"
                  class="mb-6 search"
                  label="Поиск по теме"
                  @keyup.enter="searchJoke"
          >
              <template #appendInner>
                  <va-icon         name="search" />
              </template>
          </va-input>
          <va-button @click="searchJoke">
              Найти
          </va-button>
      </div>

      <div class="jokes">

          <va-progress-circle v-if="loading" indeterminate color="primary" />
          <p v-if="!loading && jokes.length === 0">Ничего нет. Попробуйте изменить параметры поиска</p>
          <JokeCard v-for="(joke, i) in jokes" :joke="joke" :key="i"/>
      </div>
  </div>
</template>

<script>
import JokeCard from "@/components/JokeCard.vue";
import {searchJoke} from "@/api/jokes";

export default {
    name: "JokeView",
    components: {JokeCard},
    props:['token'],
    data() {
        return {
            loading: false,
            search: "",
            jokes: [
            ]
        }
    },
    methods: {
      searchJoke(){
          this.loading = true;
          searchJoke(this.token, this.search).then((jokes) =>{
            this.jokes = jokes;
            this.loading = false;
          }).catch((error) => this.$vaToast.init({ message: String(error), position: 'bottom-right' }))
      }
    },
}
</script>

<style scoped>
 .container {
     margin: 30px ;
 }
 .searching{
     width: calc(100% - 60px);
     margin: 0 30px ;
     display: flex;
     gap: 30px;
 }
 .search {
    width: 100%;
 }
 .jokes{
     display: flex;
     align-items: center;
     flex-direction: column;
     margin-top: 30px;
 }
</style>