<template>
    <div class="container">
        <va-card>
            <va-card-content>
                <va-card-title>Внимание анекдот</va-card-title>
                    <div class="title">Тема: {{joke.theme}}
                    </div>
                    <div class="text">
                        <p class="text__words">
                            {{joke.text}}
                        </p>
                    </div>
                    <p>
                        Статус: {{joke.status}}
                    </p>
                </va-card-content>
        </va-card>
    </div>
</template>

<script>
export default {
    name: "JokeCard",
    props: {
        joke :{
            type: Object,
            required: true
        }
    }
}
</script>

<style scoped>
.container{
    margin: 0 auto;
    min-width: 600px;
}
.container div {
    width: 600px;
}
.title{
    font-size: 24px;
    font-weight: bold;
    margin-bottom: 20px;
    color: rgb(180, 86, 192);
    height: 24px;
    line-height: 24px;
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