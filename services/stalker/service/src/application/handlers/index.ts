import path from 'path';
import { BaseError as DatabaseError } from 'sequelize';
import { type Context, type Env, type ErrorHandler,type Handler } from 'hono';
import { HTTPException } from 'hono/http-exception';
import { type StatusCode } from 'hono/utils/http-status';

import { unwrapError } from '@root/utils';
import { BaseError as ServiceError } from '@root/services/errors';

export function errorHandler<T extends Env>(): ErrorHandler<T> {
    return (error: unknown, ctx: Context<T>) => {
        let status: StatusCode = 500;
        let response: {
            name: string;
            message?: string;
        };

        if (typeof error === 'object' && error instanceof Error) {
            if (error instanceof DatabaseError) {
                response = {
                    name: 'DatabaseError',
                    message: unwrapError(error),
                };
            } else if (error instanceof ServiceError) {
                status = error.httpStatusCode as StatusCode;
                response = {
                    name: error.name,
                    message: unwrapError(error),
                };
            } else {
                response = {
                    name: 'UnknownError',
                    message: unwrapError(error),
                };
            }
        } else {
            response = {
                name: 'UnknownError',
            };
        }

        return ctx.json({ error: response }, status);
    };
}

export function staticFilesHandler<T extends Env>(root: string, index = 'index.html'): Handler<T> {
    return (ctx: Context<T>) => {
        let filePath: string;

        filePath = path.join(path.sep, root, ctx.req.path);
        filePath = path.normalize(filePath);

        if (!filePath.startsWith(root)) {
            throw new HTTPException(403, { message: '403 Access Denied' });
        }

        if (filePath === root) {
            filePath = path.join(filePath, path.sep, index);
        }

        const file = Bun.file(filePath);

        if (file.size === 0) {
            throw new HTTPException(404, { message: '404 Not Found' });
        }

        try {
            return ctx.body(file.stream(), 200);
        } catch (error: unknown) {
            throw new HTTPException(500, { message: '500 Internal Server Error' });
        }
    };
}
