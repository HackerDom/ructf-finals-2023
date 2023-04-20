import { type Context, type MiddlewareHandler, type Next } from 'hono';

import { User, type UserInstance } from '@root/database/models';
import { createToken, validateToken, loadPrivateKey } from '@root/utils';

export type Environment = {
    Bindings: {
        user: UserInstance | null;
    };
};

export function authTokenMiddleware(headerName: string): MiddlewareHandler<Environment> {
    return async (ctx: Context<Environment>, next: Next) => {
        const privateKey = await loadPrivateKey();

        ctx.env.user = null;

        const token = ctx.req.header(headerName);

        if (typeof token === 'string') {
            const name = await validateToken(token, privateKey);

            if (name !== null) {
                ctx.env.user = await User.findOne({ where: { name } });
            }
        }

        await next();

        const user = ctx.env.user;

        if (user !== null) {
            const token = await createToken(user.name, privateKey);

            ctx.header(headerName, token);
        }

        ctx.header('Access-Control-Expose-Headers', headerName);
    };
}