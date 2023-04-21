import { UniqueConstraintError } from 'sequelize';

import {
    AlreadyExistsError,
    LoginRequiredError,
    NoteNotFoundError,
    OwnerMismatchError,
    UnexpectedError,
    UserNotFoundError,
    ValidationError,
} from '@root/services/errors';
import { Note, User } from '@root/database/models';
import { type AppContext } from '@root/application/context';

import {
    type CreateNoteRequest,
    type CreateNoteResponse,
    type DenyNoteRequest,
    type DenyNoteResponse,
    type DestroyNoteRequest,
    type DestroyNoteResponse,
    type GetNoteRequest,
    type GetNoteResponse,
    type ShareNoteRequest,
    type ShareNoteResponse,
} from './messages';
import {
    isCreateNoteRequest,
    isDenyNoteRequest,
    isDestroyNoteRequest,
    isGetNoteRequest,
    isShareNoteRequest,
} from './validators';
import { type Method } from './index';

type NotesService = {
    get: Method<GetNoteRequest, GetNoteResponse>;
    create: Method<CreateNoteRequest, CreateNoteResponse>;
    share: Method<ShareNoteRequest, ShareNoteResponse>;
    deny: Method<DenyNoteRequest, DenyNoteResponse>;
    destroy: Method<DestroyNoteRequest, DestroyNoteResponse>;
};

const Notes: NotesService = {
    async get(ctx: AppContext, req: GetNoteRequest): Promise<GetNoteResponse> {
        if (!isGetNoteRequest(req)) {
            throw new ValidationError('invalid request message');
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

    async create(ctx: AppContext, req: CreateNoteRequest): Promise<CreateNoteResponse> {
        if (!isCreateNoteRequest(req)) {
            throw new ValidationError('invalid request message');
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

    async share(ctx: AppContext, req: ShareNoteRequest): Promise<ShareNoteResponse> {
        if (!isShareNoteRequest(req)) {
            throw new ValidationError('invalid request message');
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

        if (viewer.name !== ctx.user.name) {
            await note.addViewer(viewer);
        }

        return {};
    },

    async deny(ctx: AppContext, req: DenyNoteRequest): Promise<DenyNoteResponse> {
        if (!isDenyNoteRequest(req)) {
            throw new ValidationError('invalid request message');
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

    async destroy(ctx: AppContext, req: DestroyNoteRequest): Promise<DestroyNoteResponse> {
        if (!isDestroyNoteRequest(req)) {
            throw new ValidationError('invalid request message');
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

        await note.destroy();

        return {};
    },
};

export default Notes;
