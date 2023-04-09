<?php

class BigInteger implements ICacheable
{
    private GMP $value;
    private int $cached_bit_size = -1;

    private function __construct(GMP $value)
    {
        $this->value = $value;
    }

    public static function from(int|GMP|string $value): BigInteger
    {
        if ($value instanceof GMP)
        {
            return new BigInteger($value);
        }

        return new BigInteger(gmp_init($value));
    }

    public static function zero(): BigInteger
    {
        return new BigInteger(gmp_init(0));
    }

    public static function one(): BigInteger
    {
        return new BigInteger(gmp_init(1));
    }

    public static function random(int $bits): BigInteger
    {
        return new BigInteger(gmp_randbits($bits));
    }

    public static function from_bytes(string $bytes): BigInteger
    {
        return new BigInteger(gmp_frombytes($bytes));
    }

    public function to_bytes(): string
    {
        return gmp_tobytes($this->value);
    }

    public function bit_size(): int
    {
        if ($this->cached_bit_size < 0)
        {
            $this->cached_bit_size = gmp_bitsize($this->value);
        }

        return $this->cached_bit_size;
    }

    public function is_prime(int $checks = 10): bool
    {
        return gmp_prob_prime($this->value, $checks) > 0;
    }

    public function next_prime(): BigInteger
    {
        return new BigInteger(gmp_nextprime($this->value));
    }

    public function sign(): int
    {
        return gmp_sign($this->value);
    }

    public function neg(): BigInteger
    {
        return new BigInteger(gmp_neg($this->value));
    }

    public function add(BigInteger $other): BigInteger
    {
        return new BigInteger(gmp_add($this->value, $other->value));
    }

    public function sub(BigInteger $other): BigInteger
    {
        return new BigInteger(gmp_sub($this->value, $other->value));
    }

    public function mul(BigInteger $other): BigInteger
    {
        return new BigInteger(gmp_mul($this->value, $other->value));
    }

    public function div(BigInteger $other): BigInteger
    {
        return new BigInteger(gmp_div($this->value, $other->value));
    }

    public function mod(BigInteger $other): BigInteger
    {
        return new BigInteger(gmp_mod($this->value, $other->value));
    }

    public function pow(int $exponent): BigInteger
    {
        return new BigInteger(gmp_pow($this->value, $exponent));
    }

    public function powmod(BigInteger $exponent, BigInteger $mod): BigInteger
    {
        return new BigInteger(gmp_powm($this->value, $exponent->value, $mod->value));
    }

    public function invmod(BigInteger $mod): BigInteger
    {
        $result = gmp_invert($this->value, $mod->value);

        if ($result === false)
        {
            throw new InvalidArgumentException("BigInteger::invmod: number is not invertible");
        }

        return new BigInteger($result);
    }

    public function hash(): string
    {
        return strval($this);
    }

    public function equals(mixed $other): bool
    {
        if (!($other instanceof BigInteger))
        {
            return false;
        }

        return gmp_cmp($this->value, $other->value) === 0;
    }

    public function greater(BigInteger $other): bool
    {
        return gmp_cmp($this->value, $other->value) > 0;
    }

    public function less(BigInteger $other): bool
    {
        return gmp_cmp($this->value, $other->value) < 0;
    }

    public function hasbit(int $index): bool
    {
        return gmp_testbit($this->value, $index);
    }

    public function setbit(int $index, bool $value): void
    {
        gmp_setbit($this->value, $index, $value);
    }

    public function xor(BigInteger $other): BigInteger
    {
        return new BigInteger(gmp_xor($this->value, $other->value));
    }

    public function __clone(): void
    {
        $this->value = clone $this->value;
    }

    public function __toString(): string
    {
        return gmp_strval($this->value);
    }
}
