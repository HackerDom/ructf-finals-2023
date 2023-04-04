<?php

class IntegerRing
{
    public readonly BigInteger $modulus;

    public function __construct(BigInteger $modulus)
    {
        if ($modulus->sign() <= 0)
        {
            throw new InvalidArgumentException("modulus must be positive");
        }

        $this->modulus = $modulus;
    }

    public function contains(BigInteger $value): bool
    {
        return $value->sign() >= 0 && $value->less($this->modulus);
    }

    public function reduce(BigInteger $value): BigInteger
    {
        return $value->mod($this->modulus);
    }

    public function neg(BigInteger $value): BigInteger
    {
        return $this->reduce($value->neg());
    }

    public function add(BigInteger $left, BigInteger $right): BigInteger
    {
        return $this->reduce($left->add($right));
    }

    public function sub(BigInteger $left, BigInteger $right): BigInteger
    {
        return $this->add($left, $this->neg($right));
    }

    public function mul(BigInteger $left, BigInteger $right): BigInteger
    {
        return $this->reduce($left->mul($right));
    }

    public function div(BigInteger $left, BigInteger $right): BigInteger
    {
        return $this->mul($left, $this->invert($right));
    }

    public function invert(BigInteger $value): BigInteger
    {
        return $this->reduce($value->invmod($this->modulus));
    }

    public function pow(BigInteger $value, BigInteger $exponent): BigInteger
    {
        return $this->reduce($value->powmod($exponent, $this->modulus));
    }

    public function random_element(): BigInteger
    {
        return $this->reduce(BigInteger::random($this->modulus->bit_size()));
    }

    public function __clone(): void
    {
        $this->modulus = clone $this->modulus;
    }

    public function __toString(): string
    {
        return "Ring of integers modulo " . $this->modulus;
    }
}
