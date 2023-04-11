import { UniqueConstraintError } from 'sequelize';

import {
    AlreadyExistsError,
    InvalidArgumentError,
    LoginRequiredError,
    NoteNotFoundError,
    OwnerMismatchError,
    UnexpectedError,
    UserNotFoundError,
    ValidationError,
} from '@root/services/errors';
import { Note, User } from '@root/database/models';

import { type Context, type Message, type Method } from './index';

type GetNoteRequest = {
    title: string;
};
type GetNoteResponse = {
    title: string;
    visible: boolean;
    content?: string;
    viewers?: string[];
    owner: string;
};

type CreateNoteRequest = {
    title: string;
    visible: boolean;
    content: string;
};
type CreateNoteResponse = Message;

type ShareNoteRequest = {
    title: string;
    viewer: string;
};
type ShareNoteResponse = Message;

type DenyNoteRequest = {
    title: string;
    viewer: string;
};
type DenyNoteResponse = Message;

type DestroyNoteRequest = {
    title: string;
};
type DestroyNoteResponse = Message;

type NotesService = {
    get: Method<GetNoteRequest, GetNoteResponse>;
    create: Method<CreateNoteRequest, CreateNoteResponse>;
    share: Method<ShareNoteRequest, ShareNoteResponse>;
    deny: Method<DenyNoteRequest, DenyNoteResponse>;
    destroy: Method<DestroyNoteRequest, DestroyNoteResponse>;
};

const Notes: NotesService = {
    async get(ctx: Context, req: GetNoteRequest): Promise<GetNoteResponse> {
        if (typeof req.title !== 'string') {
            throw new InvalidArgumentError('\'title\' should be string');
        }

        const note = await Note.findOne({
            where: { title: req.title },
        });

        if (note === null) {
            throw new NoteNotFoundError('note does not exist');
        }

        const viewers = await note.getViewers()
            .then(viewers => viewers.map(viewer => viewer.name));

        const response: GetNoteResponse = {
            title: note.title,
            visible: note.visible,
            content: note.content,
            owner: note.owner.name,
            viewers: viewers,
        };

        if (ctx.user !== null && ctx.user.name === note.owner.name) {
            return response;
        }

        delete response.viewers;

        if (note.visible || ctx.user !== null && await note.hasViewer(ctx.user)) {
            return response;
        }

        delete response.content;

        return response;
    },

    async create(ctx: Context, req: CreateNoteRequest): Promise<CreateNoteResponse> {
        if (typeof req.title !== 'string') {
            throw new InvalidArgumentError('\'title\' should be string');
        }

        if (typeof req.visible !== 'boolean') {
            throw new InvalidArgumentError('\'visible\' should be boolean');
        }

        if (typeof req.content !== 'string') {
            throw new InvalidArgumentError('\'content\' should be string');
        }

        if (ctx.user === null) {
            throw new LoginRequiredError('you are not logged in');
        }

        if (!(req.title.length > 0 && req.title.length < 128) || !/\w+/.test(req.title)) {
            throw new ValidationError('invalid note title');
        }

        if (req.content.length === 0) {
            throw new ValidationError('invalid note content');
        }

        try {
            await ctx.user.createNote({
                title: req.title.toLowerCase(),
                content: req.content,
                visible: req.visible,
            });
        } catch (error) {
            if (typeof error === 'object' && error instanceof UniqueConstraintError) {
                throw new AlreadyExistsError('note already exists');
            }

            throw new UnexpectedError('failed to create note', error);
        }

        return {};
    },

    async share(ctx: Context, req: ShareNoteRequest): Promise<ShareNoteResponse> {
        if (typeof req.title !== 'string') {
            throw new InvalidArgumentError('\'title\' should be string');
        }

        if (typeof req.viewer !== 'string') {
            throw new InvalidArgumentError('\'viewer\' should be string');
        }

        if (ctx.user === null) {
            throw new LoginRequiredError('you are not logged in');
        }

        const note = ctx.user.notes.find(
            note => note.title === req.title,
        );

        if (typeof note === 'undefined') {
            throw new OwnerMismatchError('you should own this note');
        }

        const viewer = await User.findOne({ where: { name: req.viewer } });

        if (viewer === null) {
            throw new UserNotFoundError('viewer not found');
        }

        if (viewer.name !== note.owner.name) {
            await note.addViewer(viewer);
        }

        return {};
    },

    async deny(ctx: Context, req: DenyNoteRequest): Promise<DenyNoteResponse> {
        if (typeof req.title !== 'string') {
            throw new InvalidArgumentError('\'title\' should be string');
        }

        if (typeof req.viewer !== 'string') {
            throw new InvalidArgumentError('\'viewer\' should be string');
        }

        if (ctx.user === null) {
            throw new LoginRequiredError('you are not logged in');
        }

        const note = ctx.user.notes.find(
            note => note.title === req.title,
        );

        if (typeof note === 'undefined') {
            throw new OwnerMismatchError('you should own this note');
        }

        const viewer = await User.findOne({ where: { name: req.viewer } });

        if (viewer === null) {
            throw new UserNotFoundError('viewer not found');
        }

        if (await note.hasViewer(viewer)) {
            await note.removeViewer(viewer);
        }

        return {};
    },

    async destroy(ctx: Context, req: DestroyNoteRequest): Promise<DestroyNoteResponse> {
        if (typeof req.title !== 'string') {
            throw new InvalidArgumentError('\'title\' should be string');
        }

        if (ctx.user === null) {
            throw new LoginRequiredError('you are not logged in');
        }

        const note = ctx.user.notes.find(
            note => note.title === req.title,
        );

        if (typeof note === 'undefined') {
            throw new OwnerMismatchError('you should own this note');
        }

        await ctx.user.removeNote(note);

        return {};
    },
};

export default Notes;
