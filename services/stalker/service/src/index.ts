import db from '@root/database';
import app from '@root/application';

type Worker = {
    exited: Promise<number>;
};

const workersCount = Number.parseInt(Bun.env.WORKERS_COUNT ?? '8');

function time(): number {
    return Bun.nanoseconds() / 1_000_000_000; // seconds
}

function errorHandler(error: unknown): void {
    console.error('Unexpected error during execution:', error);
}

async function workerEntrypoint(): Promise<void> {
    console.log(`[*] Added new worker at ${time()} <${process.pid}>`);

    try {
        await db.authenticate();
        console.log(`[+] Successfully connected to database at ${time()} <${process.pid}>`);
    } catch (error) {
        throw new Error('failed to connect to the database', { cause: error });
    }

    try {
        await db.sync({ force: true });
        console.log(`[+] Successfully syncronized models at ${time()} <${process.pid}>`);
    } catch (error) {
        throw new Error('failed to sync models', { cause: error });
    }

    app.showRoutes();

    console.log(`[!] Starting HTTP server on 0.0.0.0:3000 at ${time()} <${process.pid}>`);

    Bun.serve({
        port: 3000,
        hostname: '0.0.0.0',

        fetch: app.fetch,
    });
}

async function coordinatorEntrypoint(): Promise<void> {
    console.log(`[*] Started Bun v${Bun.version} (commit ${Bun.revision}) at ${time()}`);

    const workers: Worker[] = [];

    for (let i = 0; i < workersCount; i++) {
        const worker = Bun.spawn({
            cmd: process.argv,
            env: {
                WORKER_ID: i.toString(),
            },

            stdout: 'inherit',
            stderr: 'inherit',

            onExit: async (worker, code, signal, error) => {
                console.log(
                    `[-] Worker ${worker.pid} exited with code ${code} and signal ${signal} at ${time()}`,
                    error,
                );
            },
        });

        workers.push(worker);
    }

    await Promise.all(
        workers.map(worker => worker.exited.catch(errorHandler)),
    );

    console.log(`[*] Application has been stopped at ${time()}`);
}

if (typeof Bun.env.WORKER_ID === 'undefined') {
    coordinatorEntrypoint().catch(errorHandler);
} else {
    workerEntrypoint().catch(errorHandler);
}
