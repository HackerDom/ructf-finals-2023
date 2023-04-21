<template>
    <div class="container">
        <div class="centers">
            <h1>ALARM!</h1>
            <h1>У нас недавно сожгли музей анекдотов</h1>
            <h1>Там были экземпляры уникальной ржаки, которые мы не в силах восстановить</h1>
            <h1>Поэтому мы создали новую соцсеть, чтобы делиться своими анекдотами с друзьями</h1>
            <h1>Из самых смешных мы создадим новый музей</h1>
            <h1>Не будьте равнодушны! Для нас это очень важно!</h1>
        </div>
        <div>

        </div>
    </div>
</template>

<script>
export default {
    name: "HomeView"
}
</script>

<style scoped>
.container {
    display: flex;
    justify-content: center;
    align-items: center;
    flex-direction: column;
    height: 80vh;
    background-image: linear-gradient(to bottom, rgba(255, 255, 255, 0) 20%, rgba(246, 246, 246, 1) 70%), url("@/assets/jokes.jpg");
    background-size: cover;
    padding: 20px;
    margin-top: -20px;
}
.centers{
    text-align: center;
    margin-top: 300px;
    margin-bottom: 15px;
}
h1 {
    font-size: 24px;
}
</style>