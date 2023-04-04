<?php

class Mask
{
    public readonly array $numbers;

    public function __construct(array $numbers)
    {
        $this->numbers = $numbers;
    }

    public static function from_matrix(Matrix $matrix): Mask
    {
        if (!$matrix->is_square())
        {
            throw new InvalidArgumentException("Mask::from_matrix: matrix must be square");
        }

        $numbers = array();

        for ($i = 0; $i < $matrix->width; $i += 1)
        {
            array_push($numbers, $matrix->get($i, $i));
        }

        return new Mask($numbers);
    }

    public function to_matrix(): Matrix
    {
        return Matrix::diagonal($this->numbers);
    }

    public function __toString(): string
    {
        $parts = array();

        for ($i = 0; $i < count($this->numbers); $i += 1)
        {
            array_push($parts, strval($this->numbers[$i]));
        }

        return "[" . implode(", ", $parts) . "]";
    }
}
