<template>
  <div class="container">
        <va-progress-circle v-if="loading" indeterminate color="primary" />
    <va-list style="max-width: 600px;">
      <va-list-label> Museum Ids that you can watch </va-list-label>
      <va-list-item
          v-for="museum_id in museum_ids"
          :key="museum_id"
          :to="{name: 'Museum', params: {id: museum_id}}"
          class="list__item"
      >
        <va-list-item-section>
            <va-card>
                <va-card-title>
                    {{ museum_id }}
                </va-card-title>
            </va-card>
        </va-list-item-section>

      </va-list-item>
    </va-list>

  </div>
</template>

<script>
import {getMuseumList} from "@/api/api";

export default {
  name: "MuseumListView",
  data() {
    return {
      museum_ids: [],
      loading: false,
    }
  },
  mounted() {
    this.loading = true
    getMuseumList().then((museum_ids) => {
      this.museum_ids = museum_ids
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