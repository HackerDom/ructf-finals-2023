import {validateResponse} from "@/api/auth";

export async function getBooksList(token){
    const response = await fetch('/api/books/', {
        method: 'GET',
        headers: {
            'Authorization' : `Token ${token}`
        }
    });
    await validateResponse(response);
    return await response.json();
}

export async function getBook(token, id){
    const response = await fetch(`/api/books/${id}/`, {
        method: 'GET',
        headers: {
            'Authorization' : `Token ${token}`
        }
    });
    await validateResponse(response);
    return await response.json();
}

export async function requestSaveBook(token,  title, text, videoFile){
    const formData = new FormData();
    formData.append('title', title);
    formData.append('text', text);
    formData.append('video', videoFile);
    const response = await fetch('/api/books/', {
        method: 'POST',
        headers: {
            'Authorization' : `Token ${token}`
        },
        body: formData
    });
    await validateResponse(response);
    return await response.json();

}