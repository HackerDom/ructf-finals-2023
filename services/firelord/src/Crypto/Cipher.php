<?php

class Cipher
{
    public static function encrypt(Key $key, Mask $mask): Matrix
    {
        $matrix = $mask->to_matrix();

        if (!$key->parameters->space->contains($matrix))
        {
            throw new InvalidArgumentException("invalid mask");
        }

        for ($i = 0; $i < count($key->keys); $i += 1)
        {
            $left = $key->parameters->space->invert($key->keys[$i]);
            $right = $key->keys[$i];

            $matrix = $key->parameters->space->mul($left, $matrix);
            $matrix = $key->parameters->space->mul($matrix, $right);
        }

        return $matrix;
    }

    public static function decrypt(Key $key, Matrix $matrix): Mask
    {
        if (!$key->parameters->space->contains($matrix))
        {
            throw new InvalidArgumentException("invalid ciphertext");
        }

        for ($i = count($key->keys) - 1; $i >= 0; $i -= 1)
        {
            $left = $key->keys[$i];
            $right = $key->parameters->space->invert($key->keys[$i]);

            $matrix = $key->parameters->space->mul($left, $matrix);
            $matrix = $key->parameters->space->mul($matrix, $right);
        }

        return Mask::from_matrix($matrix);
    }
}
