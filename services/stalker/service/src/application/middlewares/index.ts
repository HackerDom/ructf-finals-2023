import { type Context, type MiddlewareHandler, type Next } from 'hono';

import { User } from '@root/database/models';
import { type Environment } from '@root/application/context';
import { createToken, validateToken, loadPrivateKey } from '@root/utils';

export function authTokenMiddleware(headerName: string): MiddlewareHandler {
    return async (ctx: Context<Environment>, next: Next) => {
        const privateKey = await loadPrivateKey();

        ctx.set('user', null);

        const token = ctx.req.header(headerName);

        if (typeof token === 'string') {
            const name = await validateToken(token, privateKey);
            
            if (name !== null) {
                const user = await User.findOne({ where: { name } });
                ctx.set('user', user);
            }
        }

        await next();

        const user = ctx.get('user');

        if (user !== null) {
            const token = await createToken(user.name, privateKey);

            ctx.header(headerName, token);
        }

        ctx.header('Access-Control-Expose-Headers', headerName);
    };
}