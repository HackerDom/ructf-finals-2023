const TOKEN_HEADER_NAME = 'X-Token';

let token = null;

const authBox = document.getElementById('auth-box');
const menuBox = document.getElementById('menu-box');
const userBox = document.getElementById('user-box');
const noteBox = document.getElementById('note-box');

function show(element) {
    element.style.display = 'inline-block';
}

function hide(element) {
    element.style.display = 'none';
}

function handleError(obj) {
    if (typeof obj.error !== 'undefined') {
        alert(obj.error.message);
        return null;
    }

    return obj;
}

async function api_register(username, password) {
    const response = await fetch(`/users/register`, {
        method: 'POST',
        body: JSON.stringify({
            name: username,
            password: password,
        }),
    });

    token = response.headers.get('X-Token');

    return handleError(await response.json());
}

async function api_login(username, password) {
    const response = await fetch(`/users/login`, {
        method: 'POST',
        body: JSON.stringify({
            name: username,
            password: password,
        }),
    });

    token = response.headers.get('X-Token');

    return handleError(await response.json());
}

async function api_logout() {
    const response = await fetch(`/users/logout`, {
        method: 'POST',
        headers: { [TOKEN_HEADER_NAME]: token },
        body: JSON.stringify({}),
    });

    token = response.headers.get('X-Token');

    return handleError(await response.json());
}

async function api_profile(username) {
    const response = await fetch(`/users/profile/${username}`, {
        method: 'GET',
        headers: { [TOKEN_HEADER_NAME]: token },
    });

    return handleError(await response.json());
}

async function api_create_note(title, visible, content) {
    const response = await fetch(`/notes`, {
        method: 'POST',
        headers: { [TOKEN_HEADER_NAME]: token },
        body: JSON.stringify({
            title: title,
            visible: visible,
            content: content,
        }),
    });

    return handleError(await response.json());
}

async function api_share_note(title, viewer) {
    const response = await fetch(`/notes/${title}/share`, {
        method: 'POST',
        headers: { [TOKEN_HEADER_NAME]: token },
        body: JSON.stringify({
            viewer: viewer,
        }),
    });

    return handleError(await response.json());
}

async function api_destroy_note(title) {
    const response = await fetch(`/notes/${title}/destroy`, {
        method: 'POST',
        headers: { [TOKEN_HEADER_NAME]: token },
        body: JSON.stringify({}),
    });

    return handleError(await response.json());
}

async function api_get_note(title) {
    const response = await fetch(`/notes/${title}`, {
        method: 'GET',
        headers: { [TOKEN_HEADER_NAME]: token },
    });

    return handleError(await response.json());
}

function init() {
    document.getElementById('auth-register-button').onclick = async () => {
        const username = document.getElementById('username').value;
        const password = document.getElementById('password').value;

        const result = await api_register(username, password);

        if (result === null) {
            return;
        }

        hide(authBox);
        show(menuBox);

        document.getElementById('username').value = '';
        document.getElementById('password').value = '';
    };

    document.getElementById('auth-login-button').onclick = async () => {
        const username = document.getElementById('username').value;
        const password = document.getElementById('password').value;

        const result = await api_login(username, password);

        if (result === null) {
            return;
        }

        hide(authBox);
        show(menuBox);

        document.getElementById('username').value = '';
        document.getElementById('password').value = '';
    };

    document.getElementById('menu-show-note-button').onclick = async () => {
        const title = document.getElementById('menu-note-title').value;

        const note = await api_get_note(title);

        if (note === null) {
            return;
        }

        document.getElementById('note-title').value = note.title;
        document.getElementById('note-owner').value = note.owner;

        document.getElementById('note-content').value = note.content ?? '';
        document.getElementById('note-viewers').value = (note.viewers ?? []).join(',');

        hide(menuBox);
        show(noteBox);

        document.getElementById('menu-note-title').value = '';
    };

    document.getElementById('menu-show-profile-button').onclick = async () => {
        const username = document.getElementById('menu-user-name').value;

        const user = await api_profile(username);

        if (user === null) {
            return;
        }

        document.getElementById('user-name').value = user.name;
        document.getElementById('user-owned-notes').value = user.ownedNotes.join(',');

        document.getElementById('user-shared-notes').value = (user.sharedNotes ?? []).join(',');

        hide(menuBox);
        show(userBox);

        document.getElementById('menu-user-name').value = '';
    };

    document.getElementById('menu-new-note-button').onclick = async () => {
        const title = prompt('enter note title', '');
        const content = prompt('enter note content', '');
        const visible = confirm('make note visible?');

        if (title === '' || title === null || content === '' || content === null) {
            return;
        }

        const result = await api_create_note(title, visible, content);

        if (result === null) {
            return;
        }

        document.getElementById('menu-note-title').value = title;
        document.getElementById('menu-show-note-button').onclick();
    };

    document.getElementById('menu-logout-button').onclick = async () => {
        const result = await api_logout();

        if (result === null) {
            return;
        }

        hide(menuBox);
        show(authBox);
    };

    document.getElementById('user-back-button').onclick = async () => {
        hide(userBox);
        show(menuBox);
    };

    document.getElementById('note-share-button').onclick = async () => {
        const title = document.getElementById('note-title').value;

        const viewer = prompt('enter viewer', '');

        if (viewer === '' || viewer === null) {
            return;
        }

        const result = await api_share_note(title, viewer);

        if (result === null) {
            return;
        }

        hide(noteBox);

        document.getElementById('note-title').value = '';

        document.getElementById('menu-note-title').value = title;
        document.getElementById('menu-show-note-button').onclick();
    };

    document.getElementById('note-destroy-button').onclick = async () => {
        const title = document.getElementById('note-title').value;

        const result = await api_destroy_note(title);

        if (result === null) {
            return;
        }

        hide(noteBox);
        show(menuBox);

        document.getElementById('note-title').value = '';
    };

    document.getElementById('note-back-button').onclick = async () => {
        hide(noteBox);
        show(menuBox);
    };
}

async function main() {
    if (token === null || token === 'null') {
        show(authBox);
    } else {
        show(menuBox);
    }
}

init();
main();
