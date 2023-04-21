import {
    type CreateNoteRequest,
    type DenyNoteRequest,
    type DestroyNoteRequest,
    type GetNoteRequest,
    type GetUserRequest,
    type LoginUserRequest,
    type LogoutUserRequest,
    type RegisterUserRequest,
    type ShareNoteRequest,
} from './messages';
import { type Message } from './index';

export function isGetNoteRequest(obj: Message): obj is GetNoteRequest {
    if (typeof obj !== 'object' || obj === null) {
        return false;
    }

    const assertions = [
        typeof obj.title === 'string',
    ];

    return assertions.every(value => value === true);
}

export function isCreateNoteRequest(obj: Message): obj is CreateNoteRequest {
    if (typeof obj !== 'object' || obj === null) {
        return false;
    }

    const assertions = [
        typeof obj.title === 'string',
        typeof obj.visible === 'boolean',
        typeof obj.content === 'string',
    ];

    return assertions.every(value => value === true);
}

export function isShareNoteRequest(obj: Message): obj is ShareNoteRequest {
    if (typeof obj !== 'object' || obj === null) {
        return false;
    }

    const assertions = [
        typeof obj.title === 'string',
        typeof obj.viewer === 'string',
    ];

    return assertions.every(value => value === true);
}

export function isDenyNoteRequest(obj: Message): obj is DenyNoteRequest {
    if (typeof obj !== 'object' || obj === null) {
        return false;
    }

    const assertions = [
        typeof obj.title === 'string',
        typeof obj.viewer === 'string',
    ];

    return assertions.every(value => value === true);
}

export function isDestroyNoteRequest(obj: Message): obj is DestroyNoteRequest {
    if (typeof obj !== 'object' || obj === null) {
        return false;
    }

    const assertions = [
        typeof obj.title === 'string',
    ];

    return assertions.every(value => value === true);
}

export function isRegisterUserRequest(obj: Message): obj is RegisterUserRequest {
    if (typeof obj !== 'object' || obj === null) {
        return false;
    }

    const assertions = [
        typeof obj.name === 'string',
        typeof obj.password === 'string',
    ];

    return assertions.every(value => value === true);
}

export function isLoginUserRequest(obj: Message): obj is LoginUserRequest {
    if (typeof obj !== 'object' || obj === null) {
        return false;
    }

    const assertions = [
        typeof obj.name === 'string',
        typeof obj.password === 'string',
    ];

    return assertions.every(value => value === true);
}

export function isLogoutUserRequest(obj: Message): obj is LogoutUserRequest {
    if (typeof obj !== 'object' || obj === null) {
        return false;
    }

    return true;
}

export function isGetUserRequest(obj: Message): obj is GetUserRequest {
    if (typeof obj !== 'object' || obj === null) {
        return false;
    }

    const assertions = [
        typeof obj.name === 'string',
    ];

    return assertions.every(value => value === true);
}
