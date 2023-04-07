<?php

class Key
{
    public readonly Parameters $parameters;
    public readonly array $keys;

    private function __construct(Parameters $parameters, array $keys)
    {
        $this->parameters = $parameters;
        $this->keys = $keys;
    }

    public static function generate(Parameters $parameters): Key
    {
        $keys = array();

        for ($i = 0; $i < $parameters->k; $i += 1)
        {
            $key = Key::generate_single_key($parameters);
            array_push($keys, $key);
        }

        return new Key($parameters, $keys);
    }

    private static function generate_single_key(Parameters $parameters): Matrix
    {
        while (true)
        {
            $key = $parameters->space->random_element();

            if ($parameters->space->determinant($key) !== 0)
            {
                return $key;
            }
        }
    }

    public function warmup(): void
    {
        for ($i = 0; $i < count($this->keys); $i += 1)
        {
            $this->parameters->space->invert($this->keys[$i]);
        }
    }

    public function teardown(): void
    {
        $this->parameters->space->clear_caches();
    }

    public function keyspace(): string
    {
        return strval($this->parameters->space);
    }
}
