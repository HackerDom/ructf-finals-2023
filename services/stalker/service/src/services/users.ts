import { UniqueConstraintError } from 'sequelize';

import {
    AlreadyExistsError,
    InvalidArgumentError,
    InvalidCredentialsError,
    UnexpectedError,
    UserNotFoundError,
    ValidationError,
} from '@root/services/errors';
import { User } from '@root/database/models';
import { compareStrings, createHash } from '@root/utils';

import { type Context, type Message, type Method } from './index';

type RegisterUserRequest = {
    name: string;
    password: string;
};
type RegisterUserResponse = Message;

type LoginUserRequest = {
    name: string;
    password: string;
};
type LoginUserResponse = Message;

type LogoutUserRequest = Message;
type LogoutUserResponse = Message;

type GetUserRequest = {
    name: string;
};
type GetUserResponse = {
    name: string;
    ownedNotes: string[];
    sharedNotes?: string[];
};

type UsersService = {
    register: Method<RegisterUserRequest, RegisterUserResponse>;
    login: Method<LoginUserRequest, LoginUserResponse>;
    logout: Method<LogoutUserRequest, LogoutUserResponse>;
    get: Method<GetUserRequest, GetUserResponse>;
};

const Users: UsersService = {
    async register(ctx: Context, req: RegisterUserRequest): Promise<RegisterUserResponse> {
        if (typeof req.name !== 'string') {
            throw new InvalidArgumentError('\'name\' should be string');
        }

        if (typeof req.password !== 'string') {
            throw new InvalidArgumentError('\'password\' should be string');
        }

        if (!(req.name.length > 0 && req.name.length < 128) || !/\w+/.test(req.name)) {
            throw new ValidationError('invalid name');
        }

        if (req.password.length === 0) {
            throw new ValidationError('invalid password');
        }

        try {
            ctx.user = await User.create({
                name: req.name.toLowerCase(),
                password: createHash(req.password),
            });
        } catch (error) {
            if (typeof error === 'object' && error instanceof UniqueConstraintError) {
                throw new AlreadyExistsError('user already exists');
            }

            throw new UnexpectedError('failed to create user', error);
        }

        return {};
    },

    async login(ctx: Context, req: LoginUserRequest): Promise<LoginUserResponse> {
        if (typeof req.name !== 'string') {
            throw new InvalidArgumentError('\'name\' should be string');
        }

        if (typeof req.password !== 'string') {
            throw new InvalidArgumentError('\'password\' should be string');
        }

        const user = await User.findOne({ where: { name: req.name } });

        if (user === null || !compareStrings(user.password, createHash(req.password))) {
            throw new InvalidCredentialsError('invalid name or password');
        }

        ctx.user = user;

        return {};
    },

    async logout(ctx: Context, req: LogoutUserRequest): Promise<LogoutUserResponse> {
        ctx.user = null;

        return {};
    },

    async get(ctx: Context, req: GetUserRequest): Promise<GetUserResponse> {
        if (typeof req.name !== 'string') {
            throw new InvalidArgumentError('\'name\' should be string');
        }

        const user = await User.findOne({ where: { name: req.name } });

        if (user === null) {
            throw new UserNotFoundError('user does not exist');
        }

        const ownedNotes = user.notes.map(note => note.title);

        const sharedNotes = await user.getSharedNotes()
            .then(notes => notes.map(note => note.title));

        const response: GetUserResponse = {
            name: user.name,
            ownedNotes: ownedNotes,
            sharedNotes: sharedNotes,
        };

        if (ctx.user?.name === user.name) {
            return response;
        }

        delete response.sharedNotes;

        return response;
    },
};

export default Users;
