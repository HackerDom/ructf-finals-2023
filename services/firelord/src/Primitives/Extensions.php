<?php

function gmp_frombytes(string $bytes): GMP
{
    $result = 0;

    for ($i = 0; $i < strlen($bytes); $i += 1)
    {
        $result = gmp_mul($result, 256);
        $result = gmp_add($result, ord($bytes[$i]));
    }

    return $result;
}

function gmp_tobytes(GMP $value): string
{
    $value = gmp_abs($value);

    $bytes = array();

    while (gmp_sign($value) > 0)
    {
        $byte = gmp_intval(gmp_mod($value, 256));
        array_push($bytes, chr($byte));

        $value = gmp_div($value, 256);
    }

    return implode("", array_reverse($bytes));
}

function gmp_bitsize(GMP $value): int
{
    $value = gmp_abs($value);
    $bit_size = 0;

    while (gmp_scan1($value, $bit_size) >= 0)
    {
        gmp_clrbit($value, $bit_size);
        $bit_size += 1;
    }

    return $bit_size;
}

function gmp_randbits(int $bits): GMP
{
    if ($bits <= 0)
    {
        throw new InvalidArgumentException("bits must be positive");
    }

    $length = intdiv($bits + 7, 8);
    $bytes = random_bytes($length);

    $result = gmp_frombytes($bytes);

    $current = $length * 8;

    while ($current > $bits) {
        gmp_clrbit($result, $current - 1);
        $current -= 1;
    }

    return $result;
}
