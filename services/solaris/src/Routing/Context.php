<?php

class Context
{
    public readonly IStorage $storage;

    public function __construct(IStorage $storage)
    {
        $this->storage = $storage;
    }

    public function query_param(string $name): string
    {
        if (!isset($_GET) || !array_key_exists($name, $_GET))
        {
            throw Response::bad_request(sprintf("missing required param: %s", $name));
        }

        $value = $_GET[$name];

        if (!is_string($value))
        {
            throw Response::bad_request(sprintf("param is not string: %s", $name));
        }

        return $value;
    }

    public function body_content(): string
    {
        $body = file_get_contents("php://input");

        if (!is_string($body))
        {
            throw Response::bad_request("missing required body");
        }

        if (strlen($body) === 0)
        {
            throw Response::bad_request("body is empty");
        }

        return $body;
    }
}
