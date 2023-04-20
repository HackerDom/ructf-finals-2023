import { type UserInstance } from '@root/database/models';

import Users from './users';
import Notes from './notes';

export type Context = {
    user: UserInstance | null;
};

export type Message = {
    [key: string]: string | string[] | number | boolean | undefined;
};

export type Method<Req extends Message, Res extends Message> = {
    (ctx: Context, request: Req): Promise<Res>;
};

export {
    Users,
    Notes,
};
