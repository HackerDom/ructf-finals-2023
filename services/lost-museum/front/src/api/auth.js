export async function registerUser(username, password){
    const formData = new FormData();
    formData.append('username', username);
    formData.append('password', password);
    const response = await fetch('/api/register', {
        method: 'POST',
        body: formData
    });
    await validateResponse(response);
    return await response.json();
}

export async function validateResponse(response) {
    if (!response.ok) {
        if (response.json) {
            const error = await response.json();
            throw new Error(error.error);
        }
        throw new Error(response.statusText);
    }
}


export async function loginGetToken(username, password){
    const formData = new FormData();
    formData.append('username', username);
    formData.append('password', password);
    const response = await fetch('/api/login', {
        method: 'POST',
        body: formData
    });
    await validateResponse(response);
    return (await response.json()).token;
}