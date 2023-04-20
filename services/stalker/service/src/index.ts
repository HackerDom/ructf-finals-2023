import db from '@root/database';
import app from '@root/application';
import runner from '@root/runner';

const workersCount = Number.parseInt(Bun.env.WORKERS_COUNT ?? '8');

function errorHandler(error: unknown): void {
    console.error('[-] Unexpected error during execution:', error);
}

async function waitDatabase(): Promise<void> {
    const tries = 10;
    const timeout = 2 * 1000; // 2 seconds

    for (let i = 0; i < tries; i += 1) {
        try {
            await db.authenticate();
            return;
        } catch (error) {
            console.error('[-] Failed to connect to database', error);
            await Bun.sleep(timeout);
        }
    }

    throw new Error(`database is down after ${tries} tries`);
}

async function init(): Promise<void> {
    await waitDatabase();

    try {
        await db.sync();
    } catch (error) {
        throw new Error('failed to sync models', { cause: error });
    }
}

async function entrypoint(): Promise<void> {
    console.log('[*] Starting HTTP server');

    Bun.serve({
        development: false,
        maxRequestBodySize: 1 * 1024 * 1024, // 1 MiB

        fetch: app.fetch,
        error: errorHandler,
    });
}

init().then(_ => runner.run(entrypoint, workersCount));
