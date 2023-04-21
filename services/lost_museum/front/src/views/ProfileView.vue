<template>
    <div class="container">
        <div class="searching">
            <va-input
                    v-model="newUsername"
                    class="mb-6 search"
                    label="Добавить друга"
                    placeholder="Имя пользователя"
                    @keyup.enter="newFriend"
            >
            </va-input>
            <va-button @click="newFriend">
                Добавить
            </va-button>
        </div>
        <div class="block">

            <div class="list">

                <va-progress-circle v-if="loadingFriends" indeterminate color="primary" />
                <va-list style="max-width: 600px;">
                    <va-list-label> Мои друзьяшки </va-list-label>
                    <va-list-item
                            v-for="(friend, i) in friends"
                            :key="i"
                            class="list__item"
                    >

                        <va-card>
                            <va-card-content>

                                <p>
                                    {{friend}}
                                </p>
                            </va-card-content>
                        </va-card>

                    </va-list-item>
                </va-list>
            </div>
            <div class="list">

                <va-progress-circle v-if="loadingReq" indeterminate color="primary" />
                <va-list style="max-width: 600px;">
                    <va-list-label> Мои заявки </va-list-label>
                    <va-list-item
                            v-for="(request, i) in requests"
                            :key="i"
                            class="list__item"
                    >

                        <va-card>
                            <va-card-content class="carder">

                                <p>
                                    {{request}}
                                </p>
                                <va-button @click="approveRequest(request)">
                                    Принять
                                </va-button>
                            </va-card-content>
                        </va-card>
                    </va-list-item>
                </va-list>
            </div>
        </div>

    </div>
</template>

<script>

import {acceptReq, addFriend, getFriends, getRequests} from "@/api/friends";

export default {
    name: "ProfileView",
    props: ['token'],
    components: {

    },
    data(){
        return {
            newUsername: "",
            friends: [],
            requests: [],
            loadingReq: false,
            loadingFriends: false
        }
    },
    methods: {
        getFriends(){
          this.loadingFriends = true;
          getFriends(this.token).then(resp => {
              this.friends = resp
              this.loadingFriends =false;
          }).catch((error) => this.$vaToast.init({ message: String(error), position: 'bottom-right' }))
        },
        getRequests(){
            this.loadingReq = true;
            getRequests(this.token).then(resp => {
                this.requests = resp
                this.loadingReq = false
            }).catch((error) => this.$vaToast.init({ message: String(error), position: 'bottom-right' }))
        },
        newFriend(){
            addFriend(this.token, this.newUsername).then(() => {
                this.newUsername = "";
                this.getRequests()
            }).catch((error) => this.$vaToast.init({ message: String(error), position: 'bottom-right' }))
        },
        approveRequest(username){
            acceptReq(this.token, username).then(() => {
                this.getRequests();
                this.getFriends();
            }).catch((error) => this.$vaToast.init({ message: String(error), position: 'bottom-right' }))
        },
    },
    mounted() {
        this.getRequests();
        this.getFriends();
    }
}
</script>

<style scoped>
.container {
    margin: 30px ;
}
.searching{
    width: calc(100% - 60px);
    margin: 0 30px ;
    display: flex;
    gap: 30px;
}
.search {
    width: 100%;
}
.list{
    display: flex;
    align-items: center;
    flex-direction: column;
}

.block {
    display: flex;
    margin-top: 30px;
    gap: 60px;
    justify-content: center;
}
.list__item {
    width: 150px;
    margin-top: 15px;
}
.list__item div {
    width: 150px;
}
.carder{
    display: flex;
    flex-direction: column;
    gap: 20px;
}
</style>