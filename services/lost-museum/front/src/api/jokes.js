import {validateResponse} from "@/api/auth";

export async function saveNewJoke(token, theme, status, text){
    const formData = new FormData();
    formData.append('theme', theme);
    formData.append('status', status);
    formData.append('text', text);
    const response = await fetch('/api/jokes/', {
        method: 'POST',
        body: formData,
        headers: {
            'Authorization' : `Bearer ${token}`
        }
    });
    await validateResponse(response);
    return await response.json();
}

export async function getMyJokes(token){
    const response = await fetch('/api/jokes/list', {
        method: 'GET',
        headers: {
            'Authorization' : `Bearer ${token}`
        }
    });
    await validateResponse(response);
    return (await response.json()).data;
}

export async function searchJoke(token, theme){
    const response = await fetch(`/api/jokes/list/${theme}`, {
        method: 'GET',
        headers: {
            'Authorization' : `Bearer ${token}`
        }
    });
    await validateResponse(response);
    return (await response.json()).data;
}