<?php

function validate_username(string $username): void
{
    if (strlen($username) < 1 || strlen($username) > 32)
    {
        throw Response::bad_request("invalid length of username");
    }

    if (!ctype_alnum($username))
    {
        throw Response::bad_request("forbidden characters in username");
    }
}

function validate_password(string $password): void
{
    if (strlen($password) < 1 || strlen($password) > 64)
    {
        throw Response::bad_request("invalid length of password");
    }

    if (!ctype_alnum($password))
    {
        throw Response::bad_request("forbidden characters in password");
    }
}

function validate_id(string $id): void
{
    if (strlen($id) !== 32)
    {
        throw Response::bad_request("invalid length of id");
    }

    if (!ctype_xdigit($id))
    {
        throw Response::bad_request("forbidden characters in id");
    }
}

function generate_key(): Key
{
    $parameters = Parameters::generate(1024, 2, 6, 4);

    return Key::generate($parameters);
}

function generate_id(): string
{
    $bytes = random_bytes(16);

    return strtolower(bin2hex($bytes));
}
