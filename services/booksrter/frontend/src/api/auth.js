export async function loginGetToken(login, password){
    const response = await fetch('/auth/token/login', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({username: login, password})
    });
    await validateResponse(response)
    const data = await response.json();
    return data.auth_token;
}

export async function validateResponse(response) {
    if (!response.ok) {
        if (response.json) {
            const error = await response.json();
            if (error.non_field_errors) {
                throw new Error(error.non_field_errors[0]);
            }
            if (error.password) {
                throw new Error(error.password.join(' '))
            }
            if (error.email) {
                throw new Error(error.email.join(' '))
            }
            if (error.username) {
                throw new Error(error.username.join(' '))
            }
            if (error.video) {
                throw new Error(error.password.join(' '))
            }
            if (error.text) {
                throw new Error(error.text.join(' '))
            }
            if (error.title) {
                throw new Error(error.title.join(' '))
            }
            throw new Error(error.detail);
        }
        throw new Error(response.statusText);
    }
}

export async function registerUser(login, password, email){
    const response = await fetch('/auth/users/', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({username: login, password, email})
    });
    await validateResponse(response);
    return await response.json();
}

export async function logout(token){
    const response = await fetch('/auth/token/logout', {
        method: 'POST',
        headers: {
            'Authorization' : `Token ${token}`
        },
        body: token
    });
    await validateResponse(response);
}