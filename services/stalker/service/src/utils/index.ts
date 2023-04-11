import * as jose from 'jose';
import crypto from 'crypto';

import { unwrapError } from './unwrap';

const salt = 'salt1234';
const pepper = 'pepper9876';

export function createHash(text: string): string {
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

export {
    unwrapError,
};
