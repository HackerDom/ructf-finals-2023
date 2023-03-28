<template>
  <div class="container">
    <div class="centers">
      <h1>Book Arctic Vault Save</h1>
      <p> Сохрани книгу в музее арткических льдов</p>

    </div>
    <div>


    <va-stepper
        v-model="step"
        :steps="steps"
        controlsHidden
    >
      <template #step-content-0>
        <ul>
          <li>Регистрация займёт минуту</li>
          <li v-if="token === null"><router-link  to="/register">Вперёд</router-link></li>
        </ul>
      </template>
      <template #step-content-1>
        <ul>
          <li>Выбери книгу которую хочешь сохранить в музей для потомков</li>
          <li>Введи название книги</li>
          <li>Прикрепи файл</li>
          <li v-if="token !== null"><router-link  to="/book/save">Сохранить</router-link></li>
        </ul>

      </template>
      <template #step-content-2>
        <ul>
          <li>В будущем все любят смотреть короткие shorts видео</li>
          <li>Запиши короткое видео, чтобы объяснить потомкам что это за скоровище </li>
          <li>Прикрепи видео к книге </li>
        </ul>

      </template>
      <template #step-content-3>
        <ul>
          <li>Готово!</li>
          <li> Вы увековечили произведение в нашем музее.</li>
          <li>Мы сохраним их все в арктических льдах для наших потомков</li>
          <li v-if="token !== null"><router-link  to="/books">Ваш музей</router-link></li>
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
        { label: 'Зарегистрируйся' },
        { label: 'Добавь книгу' },
        { label: 'Прикрепи видео' },
        { label: 'Сохрани во льдах' }
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