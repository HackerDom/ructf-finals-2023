<?php

class CryptoController
{
    public static function encrypt(Context $ctx): Response
    {
        $key = CryptoController::load_key($ctx);
        $key->warmup();

        $data = $ctx->body_content();

        $mask = Masker::mask($key, $data);
        $ciphertext = Cipher::encrypt($key, $mask);

        $id = generate_id();

        $ctx->storage->store_ciphertext($id, $ciphertext);

        return Response::created($id);
    }

    public static function decrypt(Context $ctx): Response
    {
        $key = CryptoController::load_key($ctx);
        $key->warmup();

        $data = $ctx->body_content();
        $ciphertext = Matrix::from_json($data);

        if ($ciphertext === null)
        {
            return Response::bad_request("failed to parse ciphertext");
        }

        $mask = Cipher::decrypt($key, $ciphertext);
        $plaintext = Masker::unmask($key, $mask);

        return Response::ok($plaintext);
    }

    private static function load_key(Context $ctx): Key
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

        $key = $ctx->storage->load_key($username);

        if ($key === null)
        {
            throw Response::internal_server_error("key not found");
        }

        return $key;
    }
}
