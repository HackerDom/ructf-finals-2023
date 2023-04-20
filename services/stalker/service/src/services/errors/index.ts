export abstract class BaseError extends Error {
    declare protected code: number;

    constructor(message: string, cause?: any) {
        super(message, { cause });
        this.name = this.constructor.name;
    }

    get httpStatusCode(): number {
        return this.code;
    }
}

class BadRequestError extends BaseError { protected code = 400; }
class UnauthorizedError extends BaseError { protected code = 401; }
class AccessDeniedError extends BaseError { protected code = 403; }
class NotFoundError extends BaseError { protected code = 404; }
class ConflictError extends BaseError { protected code = 409; }
class InternalServerError extends BaseError { protected code = 500; }

export class ValidationError extends BadRequestError { }
export class LoginRequiredError extends UnauthorizedError { }
export class InvalidCredentialsError extends UnauthorizedError { }
export class OwnerMismatchError extends AccessDeniedError { }
export class NoteNotFoundError extends NotFoundError { }
export class UserNotFoundError extends NotFoundError { }
export class AlreadyExistsError extends ConflictError { }
export class UnexpectedError extends InternalServerError { }
