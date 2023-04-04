<?php

class Router
{
    private string $base;

    private array $routers;
    private array $handlers;

    public function __construct(string $base = "")
    {
        $this->base = $base;

        $this->routers = array();
        $this->handlers = array();
    }

    public function bind(Router $router): void
    {
        $this->routers[$router->base] = $router;
    }

    public function add_route(string $method, string $path, callable $handler): void
    {
        $method = strtolower($method);

        if (!array_key_exists($path, $this->handlers))
        {
            $this->handlers[$path] = array();
        }

        $this->handlers[$path][$method] = $handler;
    }

    public function route(Context $ctx, string $method, string $path): Response
    {
        $method = strtolower($method);

        $parts = explode("/", $path, 2);

        if (!is_array($parts))
        {
            throw Response::bad_request("invalid path");
        }

        if (count($parts) === 1)
        {
            if (!array_key_exists($parts[0], $this->handlers))
            {
                throw Response::not_found();
            }

            $methods = $this->handlers[$parts[0]];

            if (!array_key_exists($method, $methods))
            {
                throw Response::method_not_allowed();
            }

            return $methods[$method]($ctx);
        }

        if (!array_key_exists($parts[0], $this->routers))
        {
            throw Response::not_found();
        }

        return $this->routers[$parts[0]]->route($ctx, $method, $parts[1]);
    }
}
