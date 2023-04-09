<?php

class UserController
{
    public static function login(Context $ctx): Response
    {
        $username = $ctx->query_param("username");
        validate_username($username);

        $password = $ctx->query_param("password");
        validate_password($password);

        $saved_password = $ctx->storage->load_password($username);

        if ($saved_password === null || $password !== $saved_password)
        {
            throw Response::unauthorized("invalid username or password");
        }

        return Response::ok("logged in");
    }

    public static function register(Context $ctx): Response
    {
        $username = $ctx->query_param("username");
        validate_username($username);

        $password = $ctx->query_param("password");
        validate_password($password);

        $saved_password = $ctx->storage->load_password($username);

        if ($saved_password !== null)
        {
            throw Response::conflict("user already exists");
        }

        $ctx->storage->store_password($username, $password);

        $key = generate_key();
        $key->teardown();

        $ctx->storage->store_key($username, $key);

        return Response::created("registered");
    }
}
