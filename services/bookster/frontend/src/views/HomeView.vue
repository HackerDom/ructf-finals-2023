<template>
  <div class="container">
    <div class="centers">
      <h1>Book Arctic Vault Save</h1>
      <p> Save the book at the Arctic Ice Museum</p>

    </div>
    <div>


    <va-stepper
        v-model="step"
        :steps="steps"
        controlsHidden
    >
      <template #step-content-0>
        <ul>
          <li>Registration will take a minute</li>
          <li v-if="token === null"><router-link  to="/register">Go ahead!</router-link></li>
        </ul>
      </template>
      <template #step-content-1>
        <ul>
          <li>Choose a book that you want to keep in a museum for posterity</li>
          <li>Enter book title</li>
          <li>Drag and drop a file</li>
          <li v-if="token !== null"><router-link  to="/book/save">Go save!</router-link></li>
        </ul>

      </template>
      <template #step-content-2>
        <ul>
          <li>In the future, everyone loves to watch 'shorts' videos</li>
          <li>Record a short video to explain to posterity what kind of treasure it is </li>
          <li>Attach video to book </li>
        </ul>

      </template>
      <template #step-content-3>
        <ul>
          <li>Ready!</li>
          <li> You immortalized the work in our museum.</li>
          <li>We will save them all in the Arctic ice for our descendants.</li>
          <li v-if="token !== null"><router-link  to="/books">Your museum</router-link></li>
        </ul>

      </template>
    </va-stepper>
    </div>
  </div>
</template>

<script>
export default {
  name: "HomeView",
  props: ['token'],
  data(){
    return {
      step: 0,
      steps: [
        { label: 'Register' },
        { label: 'Add book' },
        { label: 'Attach video' },
        { label: 'Save in ice' }
      ]
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
  background-image: linear-gradient(to bottom, rgba(255, 255, 255, 0) 20%, rgba(246, 246, 246, 1) 70%), url(/src/assets/arktica.jpg);
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
</style>