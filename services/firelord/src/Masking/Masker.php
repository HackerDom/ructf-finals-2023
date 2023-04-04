<?php

class Masker
{
    public static function mask(Key $key, string $message): Mask
    {
        $number = BigInteger::from_bytes($message);
        $numbers = array($number);

        for ($i = 0; $i < $key->parameters->n - 1; $i += 1)
        {
            array_push($numbers, $key->parameters->space->ring->random_element());
        }

        return new Mask($numbers);
    }

    public static function unmask(Key $key, Mask $mask): string
    {
        $number = $mask->numbers[0];
        $message = $number->to_bytes();

        return $message;
    }
}
