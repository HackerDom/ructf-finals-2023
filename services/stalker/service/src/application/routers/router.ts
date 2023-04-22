import { Hono, type Context, type Next } from 'hono';

import { errorHandler } from '@root/application/handlers';
import { type Environment } from '@root/application/context';
import { authTokenMiddleware } from '@root/application/middlewares';

const TOKEN_HEADER_NAME = 'X-Token';

export class ServiceRouter extends Hono<Environment> {
    withAuthToken(): void {
        this.use('*', authTokenMiddleware(TOKEN_HEADER_NAME));
    }

    withErrorHandler(): void {
        this.onError(errorHandler());
    }

    withAdditionalHeaders(): void {
        this.use('*', async (ctx: Context, next: Next) => {
            await next();

            ctx.header('X-Powered-By', Bun.env.WORKER_ID);
            ctx.header('Access-Control-Expose-Headers', TOKEN_HEADER_NAME);
        });
    }
}
