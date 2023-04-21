<template>
  <div class="container">
    <va-card>
      <va-card-content>
        <va-card-title>Museum  {{id}} </va-card-title>
        <va-card-content v-if="loading">Loading...</va-card-content>
        <va-card-content v-if="!loading">
          <div class="title">{{museum.name}}</div>

        </va-card-content>
      </va-card-content>
    </va-card>
      <div class="wrapping">
      <va-list-item
              v-for="exhibit in exhibits"
              :key="exhibit.id"
              class="list__item"
      >
          <va-list-item-section>
              <va-card class="card">
                  <va-card-title>Exhibit  {{id}} </va-card-title>
                  <va-card-content>
                      <div class="title">{{exhibit.title}}</div>
                  </va-card-content>
                  <va-card-content>
                      {{ exhibit.description }}
                  </va-card-content>
              </va-card>

          </va-list-item-section>

      </va-list-item>
      </div>
  </div>
</template>

<script>
import {getMuseum, getExhibitsByMuseum} from "@/api/api";

export default {
  name: "MuseumView",
  props: ['id'],
  data(){
    return {
      museum: {},
      exhibits: [],
      loading: true,
    }
  },
  mounted() {
    this.loading = true
    getMuseum(this.id).then((museum) => {
      this.museum = museum
      this.loading = false
      getExhibitsByMuseum(this.museum.id).then((exhibits) => {
          this.exhibits = exhibits
          this.loading = false
      }).catch((err) => {
          this.$vaToast.init({ message: String(err), position: 'bottom-right' })
          this.loading = false
      })
    }).catch((err) => {
      this.$vaToast.init({ message: String(err), position: 'bottom-right' })
      this.loading = false
    })
  }

}
</script>

<style scoped>
.container{
  margin: 0 auto;
  max-width: 600px;

}
.title{
  font-size: 24px;
  font-weight: bold;
  margin-bottom: 20px;
}
.text{
  font-size: 16px;
  margin-bottom: 20px;
}
.text__title{
  font-size: 20px;
  font-weight: bold;
  margin-bottom: 20px;
  margin-top: 20px;
}
.text__words{
  margin-top: 10px;
  white-space: pre;
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
</style>