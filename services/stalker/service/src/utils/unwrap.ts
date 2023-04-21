/* eslint-disable @typescript-eslint/no-explicit-any */

export function unwrapError(error: any): string {
    const visited = new Set<any>();

    function unwrapInnerError(error: any): string | null {
        if (typeof error === 'undefined') {
            return null;
        }

        if (visited.has(error)) {
            return '...';
        }

        visited.add(error);

        let message: string | null = null;

        if (typeof error === 'object' && error instanceof Error) {
            message = error.toString();
        } else if (typeof error !== 'object' && typeof error.toString === 'function') {
            message = error.toString();
        } else if (typeof error === 'object' && typeof error[Symbol.toPrimitive] === 'function') {
            message = error[Symbol.toPrimitive]('string');
        }

        if (message === null) {
            return null;
        }

        let innerMessage: string | null = null;

        if (error.cause !== null && typeof error.cause !== 'undefined') {
            innerMessage = unwrapError(error.cause);
        }

        if (innerMessage === null) {
            return message;
        }

        return `${message} (${innerMessage})`;
    }

    return unwrapInnerError(error) ?? 'unknown error';
}
