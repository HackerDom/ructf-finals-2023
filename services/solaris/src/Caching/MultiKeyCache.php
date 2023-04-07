<?php

class MultiKeyCache
{
    private array $cache;

    public function __construct()
    {
        $this->cache = array();
    }

    public function get(ICacheable ...$keys): mixed
    {
        $hash = $this->multi_hash($keys);

        if (!array_key_exists($hash, $this->cache))
        {
            return null;
        }

        $group = $this->find_group($hash, $keys);

        if ($group === null)
        {
            return null;
        }

        return $group[count($keys)];
    }

    public function set(mixed $value, ICacheable ...$keys): void
    {
        $hash = $this->multi_hash($keys);

        if (!array_key_exists($hash, $this->cache))
        {
            $this->cache[$hash] = array();
        }

        $group = $this->find_group($hash, $keys);

        if ($group !== null)
        {
            $group[count($keys)] = $value;

            return;
        }

        array_push($this->cache[$hash], array(...$keys, $value));
    }

    public function clear(): void
    {
        $this->cache = array();
    }

    private function multi_hash(array $keys): string
    {
        if (count($keys) === 1)
        {
            return $keys[0]->hash();
        }

        return implode("_", array_map(function ($x) { return $x->hash(); }, $keys));
    }

    private function find_group(string $hash, array $keys): array|null
    {
        foreach ($this->cache[$hash] as $group)
        {
            if (count($group) !== count($keys) + 1)
            {
                continue;
            }

            $hit = true;

            for ($i = 0; $i < count($keys); $i += 1)
            {
                if (!$group[$i]->equals($keys[$i]))
                {
                    $hit = false;
                    break;
                }
            }

            if ($hit)
            {
                return $group;
            }
        }

        return null;
    }
}
