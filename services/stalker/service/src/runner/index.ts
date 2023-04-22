import { randomBytes } from "@root/utils";

type Task = () => Promise<void>;

type Worker = {
    exited: Promise<number>;
};

function time(): number {
    return Bun.nanoseconds() / 1_000_000_000; // seconds
}

async function spawnWorkers(workersCount: number): Promise<void> {
    const workers: Worker[] = [];

    for (let i = 0; i < workersCount; i++) {
        const workerId = `worker-${randomBytes(8)}`;

        const worker = Bun.spawn({
            cmd: process.argv,
            env: {
                ...Bun.env,
                WORKER_ID: workerId,
            },

            stdout: 'inherit',
            stderr: 'inherit',

            onExit: async (worker, code, signal, error) => {
                console.log(
                    `[*] Worker ${worker.pid} exited with code ${code} and signal ${signal} at ${time()}`,
                    error,
                );
            },
        });

        workers.push(worker);
    }

    await Promise.all(
        workers.map(worker => worker.exited),
    );

    console.log(`[*] Application has been stopped at ${time()}`);
}

async function run(task: Task, workersCount: number): Promise<void> {
    console.log(`[*] Started Bun v${Bun.version} (commit ${Bun.revision}) at ${time()}`);

    const workerId = Bun.env.WORKER_ID;

    if (typeof workerId === 'string') {
        console.log(`[*] Added new ${workerId} at ${time()} <${process.pid}>`);

        return task();
    }

    return spawnWorkers(workersCount);
}

export default {
    run,
};
