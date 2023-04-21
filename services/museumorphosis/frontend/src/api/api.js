export async function loginGetToken(id, password){
    const response = await fetch('/api/login', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({id: id, password: password})
    });
    await validateResponse(response)
    const data = await response.json();
    return data.token;
}

export async function registerUser(museum_name, password, email){
    const response = await fetch('/api/register', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({name: museum_name, password: password})
    });
    await validateResponse(response);
    const data = await response.json();
    return data.token;
}

export async function getMuseumList(){
    const response = await fetch('/api/museums', {
        method: 'GET',
    });
    await validateResponse(response);
    const data = await response.json();
    return data.museum_id_list;
}

export async function getMuseum(id){
    const response = await fetch(`/api/museums/${id}`, {
        method: 'GET',
    });
    await validateResponse(response);
    return await response.json();
}

export async function getExhibitsByMuseum(museumId) {
    const response = await fetch(`/api/museums/${museumId}/exhibits`, {
        method: 'GET',
    });
    await validateResponse(response);
    const data = await response.json()
    return data.exhibits;
}

export async function getExhibitsBySearch(token, search) {
    const response = await fetch(`/api/museum/exhibits?search=${search}`, {
        method: 'GET',
        headers: {
            'token': token,
        },
    });
    await validateResponse(response);
    const data = await response.json()
    return data.exhibits;
}
export async function createExhibit(token, title, desc, metadata) {
    console.log(desc)
    const response = await fetch(`/api/museum/exhibits`, {
        method: 'POST',
        headers: {
        'Content-Type': 'application/json',
        'token': token,
        },
        body: JSON.stringify({title: title, description: desc, metadata: metadata})
    });
    await validateResponse(response);
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
