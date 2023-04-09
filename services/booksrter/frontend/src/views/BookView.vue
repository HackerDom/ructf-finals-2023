<template>
  <div class="container">
    <va-card>
      <va-card-content>
        <va-card-title>Book  {{id}} </va-card-title>
        <va-card-content v-if="loading">Loading...</va-card-content>
        <va-card-content v-if="!loading">
          <div class="title">           <va-icon class="material-icons" color="background-tertiary">
            book
          </va-icon>{{book.title}}</div>
          <div class="video-reaction">
            <p>My honest videos reaction</p>
            <video
                class="video"
                controls
                :src="book.video"
                width="500"
                height="480"
            >
            </video>
          </div>

          <div class="text">
            <p class="text__title">Text:</p>
            <p class="text__words">
              {{book.text}}
            </p>
          </div>
        </va-card-content>
      </va-card-content>
    </va-card>
  </div>
</template>

<script>
import {getBook} from "@/api/books";

export default {
  name: "BookView",
  props: ['id', 'token'],
  data(){
    return {
      book: {},
      loading: true,
    }
  },
  mounted() {
    if (!this.token){
      this.$router.push('/login')
      return;
    }
    this.loading = true
    getBook(this.token, this.id).then((book) => {
      this.book = book
      this.loading = false
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
.video {
  margin-top: 10px;
}
.text__words{
  margin-top: 10px;
  white-space: pre;
}
</style>