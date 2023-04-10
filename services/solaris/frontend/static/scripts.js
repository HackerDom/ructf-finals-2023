const registerButton = document.getElementById('register-button');
const loginButton = document.getElementById('login-button');
const logoutButton = document.getElementById('logout-button');

const usernameArea = document.getElementById('username');
const passwordArea = document.getElementById('password');

const encryptButton = document.getElementById('encrypt-button');
const decryptButton = document.getElementById('decrypt-button');

const plaintextArea = document.getElementById('plaintext');
const ciphertextIdArea = document.getElementById('ciphertext-id');

const authBox = document.getElementById('auth-box');
const mainBox = document.getElementById('main-box');

function show(element) {
    element.style.display = 'block';
}

function hide(element) {
    element.style.display = 'none';
}

async function do_register(username, password) {
    const url = `/api/user/register?username=${username}&password=${password}`;

    const response = await fetch(url, { method: 'POST' });
    const text = (await response.text()).trim();

    if (response.status === 201) {
        return true;
    }

    alert(`failed to register: ${text}`);

    return false;
}

async function do_login(username, password, silent = false) {
    const url = `/api/user/login?username=${username}&password=${password}`;

    const response = await fetch(url, { method: 'POST' });
    const text = (await response.text()).trim();

    if (response.status === 200) {
        return true;
    }

    if (!silent) {
        alert(`failed to login: ${text}`);
    }

    return false;
}

async function do_encrypt(username, password, plaintext) {
    const url = `/api/crypto/encrypt?username=${username}&password=${password}`;

    const response = await fetch(url, { method: 'POST', body: plaintext });
    const text = (await response.text()).trim();

    if (response.status === 201) {
        return text;
    }

    alert(`failed to encrypt: ${text}`);

    return false;
}

async function do_decrypt(username, password, ciphertext_id) {
    const url = `/api/storage/ciphertext?id=${ciphertext_id}`;

    const response = await fetch(url, { method: 'GET' });
    const text = (await response.text()).trim();

    if (response.status === 200) {
        const url = `/api/crypto/decrypt?username=${username}&password=${password}`;

        const response = await fetch(url, { method: 'POST', body: text });
        const plaintext = (await response.text()).trim();

        if (response.status === 200) {
            return plaintext;
        }

        alert(`failed to decrypt: ${plaintext}`);

        return false;
    }

    alert(`failed to get ciphertext: ${text}`);

    return false;
}

registerButton.onclick = async function() {
    const username = usernameArea.value;
    const password = passwordArea.value;

    const result = await do_register(username, password);

    if (result) {
        localStorage.setItem('username', username);
        localStorage.setItem('password', password);

        hide(authBox);
        show(mainBox);

        usernameArea.value = '';
        passwordArea.value = '';
    }
};

loginButton.onclick = async function() {
    const username = usernameArea.value;
    const password = passwordArea.value;

    const result = await do_login(username, password);

    if (result) {
        localStorage.setItem('username', username);
        localStorage.setItem('password', password);

        hide(authBox);
        show(mainBox);

        usernameArea.value = '';
        passwordArea.value = '';
    }
};

logoutButton.onclick = async function() {
    localStorage.removeItem('username');
    localStorage.removeItem('password');

    hide(mainBox);
    show(authBox);

    plaintextArea.value = '';
    ciphertextIdArea.value = '';
};

encryptButton.onclick = async function() {
    const username = localStorage.getItem('username');
    const password = localStorage.getItem('password');

    if (username === null || password === null) {
        alert('error: please login / register');

        hide(mainBox);
        show(authBox);

        return;
    }

    const result = await do_encrypt(username, password, plaintextArea.value);

    if (result !== false) {
        ciphertextIdArea.value = result;
        plaintextArea.value = '';
    }
}

decryptButton.onclick = async function() {
    const username = localStorage.getItem('username');
    const password = localStorage.getItem('password');

    if (username === null || password === null) {
        alert('error: please login / register');

        hide(mainBox);
        show(authBox);

        return;
    }

    const result = await do_decrypt(username, password, ciphertextIdArea.value);

    if (result !== false) {
        plaintextArea.value = result;
        ciphertextIdArea.value = '';
    }
}

async function main() {
    const username = localStorage.getItem('username');
    const password = localStorage.getItem('password');

    if (username === null || password === null) {
        hide(mainBox);
        show(authBox);

        return;
    }

    const result = await do_login(username, password, true);

    if (result) {
        hide(authBox);
        show(mainBox);
    } else {
        localStorage.removeItem('username');
        localStorage.removeItem('password');

        hide(mainBox);
        show(authBox);
    }
}

const waitLoader = function() {
    const loader = document.getElementById('loader');

    if (loader.hidden) {
        return main();
    }
    
    setTimeout(waitLoader, 100);
};

waitLoader();
