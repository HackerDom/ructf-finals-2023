<template>
  <div class="container">
    <va-card>
      <va-card-title>
        <h2>Save exhibit</h2>
      </va-card-title>
      <va-card-content >
        <va-form ref="form" tag="form" @submit.prevent="onSubmit" style="max-width: 500px;">
          <va-input class="input"
              v-model="title"
              label="Title"
              style="width: 100%"
              :rules="[(value) => (value && value.length > 0) || 'Enter title']"
          />
            <va-input class="input"
                    v-model="description"
                    label="Description"
                    style="width: 100%"
                    :rules="[(value) => (value && value.length > 0) || 'Enter description']"
            />
            <va-input clsas="input"
                    v-model="metadata"
                    label="Metadata"
                    style="width: 100%"
                    :rules="[(value) => (value && value.length > 0) || 'Enter metadata']"
            />
          <div class="container">
            <va-button
                type="submit"
                class="mt-3"
            >
              Save
            </va-button>
          </div>
        </va-form>
      </va-card-content>
    </va-card>
  </div>
</template>

<script>
import {createExhibit} from "@/api/api";
import router from "@/router";

export default {
  name: "SaveExhibitView",
  props:['token'],
  data() {
    return {
      title: "",
      description: "",
      metadata: ""
    };
  },
  methods: {
    onSubmit(){
      createExhibit(this.token, this.title, this.description, this.metadata)
          .then(() => {
            router.push('/')
          })
          .catch((error) => {
            this.$vaToast.init({ message: String(error), position: 'bottom-right' })
          });
    }
  },
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
.input {
    margin-bottom: 10px;
}
h2{

}
</style>