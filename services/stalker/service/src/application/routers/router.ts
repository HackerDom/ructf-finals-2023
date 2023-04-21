import { Hono } from 'hono';

import { errorHandler } from '@root/application/handlers';
import { type Environment, authTokenMiddleware } from '@root/application/middlewares';

const TOKEN_HEADER_NAME = 'X-Token';

export class ServiceRouter extends Hono<Environment> {
    withAuthToken(): void {
        this.use('*', authTokenMiddleware(TOKEN_HEADER_NAME));
    }

    withErrorHandler(): void {
        this.onError(errorHandler());
    }
}
