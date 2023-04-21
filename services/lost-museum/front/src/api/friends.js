import {validateResponse} from "@/api/auth";


export async function addFriend(token, username){
    const formData = new FormData();
    formData.append('to', username);
    const response = await fetch('/api/friends/request', {
        method: 'POST',
        headers: {
            'Authorization' : `Bearer ${token}`
        },
        body: formData
    });
    await validateResponse(response);
    return await response.json();
}

export async function getFriends(token){
    const response = await fetch('/api/friends/', {
        method: 'GET',
        headers: {
            'Authorization' : `Bearer ${token}`
        }
    });
    await validateResponse(response);
    return (await response.json()).data;
}

export async function getRequests(token){
    const response = await fetch('/api/friends/requests', {
        method: 'GET',
        headers: {
            'Authorization' : `Bearer ${token}`
        }
    });
    await validateResponse(response);
    return (await response.json()).data;
}

export async function acceptReq(token, username){
    const formData = new FormData();
    formData.append('from', username);
    const response = await fetch('/api/friends/accept', {
        method: 'POST',
        headers: {
            'Authorization' : `Bearer ${token}`
        },
        body: formData
    });
    await validateResponse(response);
    return await response.json();
}