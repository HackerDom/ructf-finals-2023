<?php

class MixedProxy implements ICacheable
{
    private mixed $value;

    private function __construct(mixed $value)
    {
        $this->value = $value;
    }

    public static function create(mixed $value): MixedProxy
    {
        return new MixedProxy($value);
    }

    public function hash(): string
    {
        return strval($this->value);
    }

    public function equals(mixed $other): bool
    {
        if ($other instanceof MixedProxy)
        {
            return $this->value === $other->value;
        }

        return $this->value === $other;
    }
}
