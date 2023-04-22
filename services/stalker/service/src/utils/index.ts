import * as jose from 'jose';
import crypto from 'crypto';

import { unwrapError } from './unwrap';

export function createHash(text: string): string {
    const salt = 'salt1234';
    const pepper = 'pepper9876';

    return Bun.sha(salt + text + pepper, 'hex');
}

export function compareStrings(str1: string, str2: string): boolean {
    return crypto.timingSafeEqual(
        Buffer.from(str1),
        Buffer.from(str2),
    );
}

export async function createToken(data: string, key: string): Promise<string> {
    const secret = new TextEncoder().encode(key);

    return new jose.SignJWT({ data })
        .setProtectedHeader({ alg: 'HS256' })
        .sign(secret);
}

export async function validateToken(token: string, key: string): Promise<string | null> {
    const secret = new TextEncoder().encode(key);

    try {
        const result = await jose.jwtVerify(token, secret, { algorithms: ['HS256'] });

        if (typeof result.payload.data === 'string') {
            return result.payload.data;
        }
    } catch {
        // noop
    }

    return null;
}

export async function parseJson<T = any>(stream: ReadableStream<Uint8Array> | null): Promise<T> {
    if (stream === null) {
        return {} as T;
    }

    try {
        return Bun.readableStreamToJSON(stream);
    } catch (error: unknown) {
        throw new Error('failed to parse json', { cause: error });
    }
}

export function randomBytes(length: number): string {
    return crypto.randomBytes(length).toString('hex');
}

export async function loadPrivateKey(): Promise<string> {
    const keyPath = '/tmp/secrets/key.txt';

    const file = Bun.file(keyPath);

    if (file.size === 0) {
        const key = randomBytes(16);

        const length = await Bun.write(file, key);

        if (length < key.length) {
            throw new Error('failed to write private key');
        }
    }

    const key = await Bun.readableStreamToText(file.stream());

    if (key.length === 0) {
        throw new Error('failed to read private key');
    }

    return key;
}

export {
    unwrapError,
};
