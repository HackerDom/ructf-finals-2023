import { type Context as HonoContext } from 'hono';

import { type UserInstance } from '@root/database/models';

export type AppContext = {
    user: UserInstance | null;
};

export type Environment = {
    Variables: AppContext;
};

type Handler<T> = (ctx: HonoContext<Environment>) => Promise<T>;
type AppHandler<T> = (ctx: HonoContext<Environment>, appCtx: AppContext) => Promise<T>;

export function withAppContext<T>(handler: AppHandler<T>): Handler<T> {
    return async function (ctx: HonoContext<Environment>): Promise<T> {
        const appCtx = {
            user: ctx.get('user'),
        };

        const response = await handler(ctx, appCtx);

        ctx.set('user', appCtx.user);

        return response;
    };
}
