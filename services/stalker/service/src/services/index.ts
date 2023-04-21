import { type AppContext } from '@root/application/context';

import Users from './users';
import Notes from './notes';

export type Message = {
    [key: string]: string | string[] | number | boolean | undefined;
};

export type Method<Req extends Message, Res extends Message> = {
    (ctx: AppContext, request: Req): Promise<Res>;
};

export {
    Users,
    Notes,
};
