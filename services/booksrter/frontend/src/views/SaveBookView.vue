<template>
  <div class="container">
    <va-card>
      <va-card-title>
        <h2>Сохранить книгу</h2>
      </va-card-title>
      <va-card-content >
        <va-form ref="form" tag="form" @submit.prevent="onSubmit" style="max-width: 500px;">
          <va-input
              v-model="title"
              label="Название"
              style="width: 100%"
              :rules="[(value) => (value && value.length > 0) || 'Введите название']"
          />
          <va-list-label class="ml-3" style="text-align: left;">
            Книга
          </va-list-label>
          <va-file-upload
              v-model="text"
              type="single"
              :upload-button-text="'Выбрать книгу'"
              dropzone
              file-types="txt"
          />
          <va-list-label class="ml-3" style="text-align: left;">
            Видео с реакцией
          </va-list-label>
          <va-file-upload
              v-model="file"
              type="single"
              :upload-button-text="'Выбрать видео'"
              dropzone
              file-types="mp4,avi,webm"
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
import {requestSaveBook} from "@/api/books";
import router from "@/router";

export default {
  name: "SaveBookView",
  props:['token'],
  data() {
    return {
      file: undefined,
      title: "",
      text: undefined
    };
  },
  methods: {
    onSubmit(){
      if (!this.file){
        this.$vaToast.init({ message: 'Выберите видео', position: 'bottom-right' })
        return
      }
      if (!this.text){
        this.$vaToast.init({ message: 'Выберите файл с текстом книги', position: 'bottom-right' })
        return
      }
      requestSaveBook(this.token, this.title, this.text, this.file)
          .then((response) => {
            router.push('/books/' + response.uid)
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
</style>