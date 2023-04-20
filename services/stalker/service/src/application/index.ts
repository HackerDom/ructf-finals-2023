import { Hono } from 'hono';

import { staticFilesHandler } from '@root/application/handlers';
import { notesRouter, usersRouter } from '@root/application/routers';

const STATIC_ROOT = Bun.env.STATIC_ROOT ?? process.cwd();

class Aplication extends Hono {
    useStaticFiles(rootDir: string): void {
        this.get('*', staticFilesHandler(rootDir));
    }
}

const app = new Aplication();

app.route('/users', usersRouter);
app.route('/notes', notesRouter);

app.useStaticFiles(STATIC_ROOT);

export default app;
