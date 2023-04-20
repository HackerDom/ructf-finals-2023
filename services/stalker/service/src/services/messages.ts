import { type Message } from './index';

export type GetNoteRequest = {
    title: string;
};

export type GetNoteResponse = {
    title: string;
    visible: boolean;
    content?: string;
    viewers?: string[];
    owner: string;
};

export type CreateNoteRequest = {
    title: string;
    visible: boolean;
    content: string;
};

export type CreateNoteResponse = Message;

export type ShareNoteRequest = {
    title: string;
    viewer: string;
};

export type ShareNoteResponse = Message;

export type DenyNoteRequest = {
    title: string;
    viewer: string;
};

export type DenyNoteResponse = Message;

export type DestroyNoteRequest = {
    title: string;
};

export type DestroyNoteResponse = Message;

export type RegisterUserRequest = {
    name: string;
    password: string;
};

export type RegisterUserResponse = Message;

export type LoginUserRequest = {
    name: string;
    password: string;
};

export type LoginUserResponse = Message;

export type LogoutUserRequest = Message;

export type LogoutUserResponse = Message;

export type GetUserRequest = {
    name: string;
};

export type GetUserResponse = {
    name: string;
    ownedNotes: string[];
    sharedNotes?: string[];
};
