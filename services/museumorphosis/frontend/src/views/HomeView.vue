<template>
  <div class="container">
    <div >
        <div v-if="token === null" class="centers">
            <h1>Welcome to the "Museum of Museums"</h1>
            <p>Here you can create your own museum and place your exhibits in it</p>
            <p></p>
        </div>

        <div v-if="token !== null" class="searching">
            <va-input
                    v-model="search"
                    class="mb-6 search"
                    label="Search by name"
                    @keyup.enter="searchExhibit"
            >
                <template #appendInner>
                    <va-icon         name="search" />
                </template>
            </va-input>
            <va-button @click="searchExhibit">
                Найти
            </va-button>
        </div>

        <div v-if="token !== null" class="wrapping">
            <va-list-item
                    v-for="exhibit in exhibits"
                    :key="exhibit.id"
                    class="list__item"
            >
                <va-list-item-section>
                    <va-card class="card">
                        <va-card-title>Exhibit  {{exhibit.id}} </va-card-title>
                        <va-card-content>
                            <div class="title">
                                        {{exhibit.title}}
                            </div>
                        </va-card-content>
                        <va-card-content>
                                    <strong>Description:</strong><br>{{ exhibit.description }}
                        </va-card-content>
                        <va-card-content>
                            <strong>Metadata:</strong><br>{{ exhibit.metadata }}
                        </va-card-content>
                    </va-card>

                </va-list-item-section>

            </va-list-item>
        </div>
    </div>
  </div>
</template>

<script>

import {getExhibitsBySearch} from "@/api/api";

export default {
  name: "HomeView",
  props: ['token'],
  data(){
    return {
        loading: false,
        search: "",
        exhibits: []
    }
  },
  methods: {
      searchExhibit() {
          this.loading = true;
          getExhibitsBySearch(this.token, this.search).then((exhibits) =>{
              this.exhibits = exhibits;
              this.loading = false;
          }).catch((error) => this.$vaToast.init({ message: String(error), position: 'bottom-right' }))
      }
  },
    mounted() {
        if (this.token !== null) {
            getExhibitsBySearch(this.token, this.search).then((exhibits) => {
                this.exhibits = exhibits;
                this.loading = false;
            }).catch((error) => this.$vaToast.init({message: String(error), position: 'bottom-right'}))
        }
    }

}
</script>

<style scoped>
.container {
  display: flex;
  justify-content: center;
  align-items: center;
  flex-direction: column;
  height: 80vh;
  background-image: linear-gradient(to bottom, rgba(255, 255, 255, 0) 20%, rgba(246, 246, 246, 1) 70%));
  background-size: cover;
  padding: 20px;
  margin-top: -20px;
}
.centers{
  text-align: center;
  margin-top: 15px;
  margin-bottom: 15px;
}
ul:not([class^=va])>li {
  list-style: none;
  padding: 0 1rem;
  position: relative;
}
ul:not([class^=va])>li:before {
  background-color: var(--va-li-background);
  border-radius: 50%;
  content: "";
  height: 0.5rem;
  left: 0;
  margin-top: 0.5rem;
  position: absolute;
  top: 0;
  width: 0.5rem;
}
h1 {
  font-size: 2em;
}
.wrapping {
    margin-top: 10px;
    flex-wrap: wrap;
    display: flex;
}
.card {
    width: 30vw;
    margin: 10px;
}
.searching{
    width: 95vw;
    margin: 0 30px ;
    display: flex;
    gap: 30px;
}
.search {
    width: 100%;
}
.title{
    font-size: 24px;
    font-weight: bold;
    margin-bottom: 20px;
}
</style>