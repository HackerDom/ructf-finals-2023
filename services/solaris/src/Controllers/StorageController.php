<?php

class StorageController
{
    public static function ciphertext(Context $ctx): Response
    {
        $id = $ctx->query_param("id");
        validate_id($id);

        $ciphertext = $ctx->storage->load_ciphertext($id);

        if ($ciphertext === null)
        {
            return Response::not_found("ciphertext not found");
        }

        return Response::ok($ciphertext->to_json());
    }

    public static function keyspace(Context $ctx): Response
    {
        $username = $ctx->query_param("username");
        validate_username($username);

        $key = $ctx->storage->load_key($username);

        if ($key === null)
        {
            return Response::not_found("key not found");
        }

        return Response::ok($key->keyspace());
    }
}
