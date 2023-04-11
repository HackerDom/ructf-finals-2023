import { Hono } from 'hono';

import { errorHandler } from '@root/application/handlers';
import { type Environment, authCookieMiddleware } from '@root/application/middlewares';

export class ServiceRouter extends Hono<Environment> {
    private static cookieName = 'token';

    withAuthCookie(cookieName: string = ServiceRouter.cookieName): void {
        this.use('*', authCookieMiddleware(cookieName));
    }

    withErrorHandler(): void {
        this.onError(errorHandler());
    }
}
