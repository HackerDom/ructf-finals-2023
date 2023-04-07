<?php

class Response extends Exception
{
    private function __construct(int $code, string $message)
    {
        $this->code = $code;
        $this->message = $message;
    }

    public static function ok(string $message = "ok"): Response
    {
        return new Response(200, $message);
    }

    public static function created(string $message = "created"): Response
    {
        return new Response(201, $message);
    }

    public static function bad_request(string $message = "bad request"): Response
    {
        return new Response(400, $message);
    }

    public static function unauthorized(string $message = "unauthorized"): Response
    {
        return new Response(401, $message);
    }

    public static function forbidden(string $message = "forbidden"): Response
    {
        return new Response(403, $message);
    }

    public static function not_found(string $message = "not found"): Response
    {
        return new Response(404, $message);
    }

    public static function method_not_allowed(string $message = "method not allowed"): Response
    {
        return new Response(405, $message);
    }

    public static function conflict(string $message = "conflict"): Response
    {
        return new Response(409, $message);
    }

    public static function internal_server_error(string $message = "internal server error"): Response
    {
        return new Response(500, $message);
    }
}
