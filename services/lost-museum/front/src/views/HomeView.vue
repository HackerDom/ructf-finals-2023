<template>
    <div class="container">
        <div class="centers">
            <h1>Восстанови анекдоты</h1>
            <p> Пожалуйста</p>
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
    margin-top: 15px;
    margin-bottom: 15px;
}
</style>