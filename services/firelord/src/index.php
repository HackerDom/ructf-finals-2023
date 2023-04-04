<?php

spl_autoload_register(function () {
    $files = glob("./*/*.php");

    foreach ($files as $file)
    {
        include_once $file;
    }
});

function get_request_method(): string
{
    if (!isset($_SERVER) || !array_key_exists("REQUEST_METHOD", $_SERVER))
    {
        throw Response::bad_request("method does not exist");
    }

    return $_SERVER["REQUEST_METHOD"];
}

function get_url_path(): string
{
    if (!isset($_SERVER) || !array_key_exists("REQUEST_URI", $_SERVER))
    {
        throw Response::bad_request("url does not exist");
    }

    $url = parse_url($_SERVER["REQUEST_URI"], PHP_URL_PATH);

    if (!is_string($url))
    {
        throw Response::bad_request("invalid url");
    }

    $url = trim($url, "/");

    return $url;
}

function get_router(): Router
{
    $crypto_router = new Router("crypto");
    $crypto_router->add_route("POST", "encrypt", "CryptoController::encrypt");
    $crypto_router->add_route("POST", "decrypt", "CryptoController::decrypt");

    $storage_router = new Router("storage");
    $storage_router->add_route("GET", "ciphertext", "StorageController::ciphertext");
    $storage_router->add_route("GET", "keyspace", "StorageController::keyspace");

    $user_router = new Router("user");
    $user_router->add_route("POST", "login", "UserController::login");
    $user_router->add_route("POST", "register", "UserController::register");

    $api_router = new Router("api");
    $api_router->bind($crypto_router);
    $api_router->bind($storage_router);
    $api_router->bind($user_router);

    $router = new Router();
    $router->bind($api_router);

    return $router;
}

function main(): void
{
    try
    {
        $storage = new DiskStorage("/tmp/storage/");
        $ctx = new Context($storage);

        $router = get_router();

        $method = get_request_method();
        $path = get_url_path();

        $response = $router->route($ctx, $method, $path);

        http_response_code($response->getCode());
        echo $response->getMessage();
    }
    catch (Response $response)
    {
        http_response_code($response->getCode());
        echo $response->getMessage();
    }
    catch (Exception $exception)
    {
        printf("Uncaught exception: %s" . PHP_EOL, $exception->getMessage());

        http_response_code(500);
        echo $exception->getMessage();
    }
}

error_reporting(E_ERROR);
main();
