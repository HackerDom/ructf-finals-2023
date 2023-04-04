<?php

function sum(BigInteger ...$numbers): BigInteger
{
    $result = BigInteger::zero();

    foreach ($numbers as $number)
    {
        $result = $result->add($number);
    }

    return $result;
}

function mul(BigInteger ...$numbers): BigInteger
{
    $result = BigInteger::one();

    foreach ($numbers as $number)
    {
        $result = $result->mul($number);
    }

    return $result;
}

function pad(string $bytes, int $length): string
{
    if (strlen($bytes) >= $length)
    {
        return $bytes;
    }

    $parts = array();
    $total_length = strlen($bytes);

    while (true)
    {
        $chunk = random_bytes($length);
        $index = 0;

        while ($total_length < $length - 1 && $index < $length)
        {
            $byte = $chunk[$index];
            $index += 1;

            if ($byte === "\x00")
            {
                continue;
            }

            array_push($parts, $byte);
            $total_length += 1;
        }

        if ($total_length === $length - 1)
        {
            array_push($parts, "\x00");
            array_push($parts, $bytes);

            return implode("", $parts);
        }
    }
}

function unpad(string $bytes): string
{
    $position = strpos($bytes, "\x00");

    if ($position === false)
    {
        return $bytes;
    }

    return substr($bytes, $position + 1);
}
