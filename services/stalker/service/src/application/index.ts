import { Hono } from 'hono';

import { notesRouter, usersRouter } from '@root/application/routers';
import { staticFilesHandler } from '@root/application/handlers';

class Aplication extends Hono {
    private static rootDir = Bun.env.STATIC_ROOT ?? process.cwd();

    withStaticFiles(rootDir: string = Aplication.rootDir): void {
        this.get('*', staticFilesHandler(rootDir));
    }
}

const app = new Aplication();

app.withStaticFiles();

app.route('/users', usersRouter);
app.route('/notes', notesRouter);

export default app;
