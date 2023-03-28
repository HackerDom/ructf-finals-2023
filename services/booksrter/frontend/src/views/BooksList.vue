<template>
  <div class="container">
        <va-progress-circle v-if="loading" indeterminate color="primary" />
    <va-list style="max-width: 600px;">
      <va-list-label> Книги сохранённые вами в музей </va-list-label>
      <va-list-item
          v-for="book in books"
          :key="book.uid"
          :to="{name: 'Book', params: {id: book.uid}}"
          class="list__item"
      >
        <va-list-item-section avatar>
          <va-avatar>
            <img v-if="book.video_preview" :src="book.video_preview">
            <img v-else src="@/assets/lolo.jpg">
          </va-avatar>
        </va-list-item-section>

        <va-list-item-section>
          <va-list-item-label>
            {{ book.title }}
          </va-list-item-label>

        </va-list-item-section>

        <va-list-item-section icon>
          <va-icon class="material-icons" color="background-tertiary">
            book
          </va-icon>
        </va-list-item-section>
      </va-list-item>
    </va-list>

  </div>
</template>

<script>
import {getBooksList} from "@/api/books";

export default {
  name: "BooksList",
  props: ['token'],
  data() {
    return {
      books: [],
      loading: false,
    }
  },
  mounted() {
    if (!this.token){
      this.$router.push('/login')
      return;
    }
    this.loading = true
    getBooksList(this.token).then((books) => {
      this.books = books
      this.loading = false
    }).catch((err) => {
      this.$vaToast.init({ message: String(err), position: 'bottom-right' })
      this.loading = false
    })
  }
}
</script>

<style scoped>
.list__item + .list__item {
  margin-top: 20px;
}
img {
  width: 100%;
  height: 100%;
  object-fit: cover ;
}
.container{
  padding: 20px;
  display: flex;
  flex-direction: column;
  align-items: center;
}

</style>