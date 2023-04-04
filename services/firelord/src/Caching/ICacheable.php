<?php

interface ICacheable
{
    public function hash(): string;
    public function equals(mixed $other): bool;
}
