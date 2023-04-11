import { type Context, type MiddlewareHandler, type Next } from 'hono';

import { User, type UserInstance } from '@root/database/models';
import { createToken, validateToken } from '@root/utils';

export type Environment = {
    Bindings: {
        user: UserInstance | null;
    };
};

export function authCookieMiddleware(cookieName: string): MiddlewareHandler<Environment> {
    const privateKey = 'abeba';

    return async (ctx: Context<Environment>, next: Next) => {
        ctx.env.user = null;

        const token = ctx.req.cookie(cookieName);

        if (typeof token !== 'undefined') {
            const name = await validateToken(token, privateKey);

            if (name !== null) {
                ctx.env.user = await User.findOne({ where: { name } });
            }
        }

        await next();

        const user = ctx.env.user;

        if (user !== null) {
            const token = await createToken(user.name, privateKey);

            ctx.cookie(cookieName, token, { httpOnly: true });
        } else {
            ctx.cookie(cookieName, '', { maxAge: 0, expires: new Date() });
        }
    };
}
