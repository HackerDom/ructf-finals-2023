<?php

class Matrix implements ICacheable
{
    public readonly int $width;
    public readonly int $height;

    private array $values;

    private BigInteger $hash_value;

    private function __construct(int $width, int $height, array $values, BigInteger $hash = null)
    {
        $this->width = $width;
        $this->height = $height;

        $this->values = $values;

        if ($hash !== null)
        {
            $this->hash_value = $hash;
        }
        else
        {
            $this->update_hash();
        }
    }

    private function update_hash(): void
    {
        $hash = BigInteger::zero();

        for ($i = 0; $i < count($this->values); $i += 1)
        {
            $hash = $hash->xor($this->values[$i]);
        }

        $this->hash_value = $hash;
    }

    public static function zero(int $width, int $height): Matrix
    {
        if ($width <= 0 || $height <= 0)
        {
            throw new InvalidArgumentException("width and height must be positive");
        }

        $values = array();

        for ($i = 0; $i < $width * $height; $i += 1)
        {
            array_push($values, BigInteger::zero());
        }

        return new Matrix($width, $height, $values, BigInteger::zero());
    }

    public static function diagonal(array $values): Matrix
    {
        if (count($values) === 0)
        {
            throw new InvalidArgumentException("values must not be empty");
        }

        $matrix = Matrix::zero(count($values), count($values));

        for ($i = 0; $i < count($values); $i += 1)
        {
            if (!($values[$i] instanceof BigInteger))
            {
                throw new InvalidArgumentException("item must be an instance of BigInteger");
            }

            $matrix->set($i, $i, $values[$i]);
        }

        return $matrix;
    }

    public static function from_json(string $json): Matrix|null
    {
        $data = json_decode($json, false, 3, JSON_BIGINT_AS_STRING);

        if ($data === null)
        {
            return null;
        }

        if (!is_array($data) || !array_is_list($data) || count($data) === 0)
        {
            return null;
        }

        $values = array();

        $height = count($data);
        $width = -1;

        for ($i = 0; $i < $height; $i += 1)
        {
            $row = $data[$i];

            if ($width < 0) {
                $width = count($row);
            }

            if (count($row) !== $width)
            {
                return null;
            }

            foreach ($row as $value)
            {
                if (!is_string($value) && !is_integer($value))
                {
                    return null;
                }

                array_push($values, BigInteger::from($value));
            }
        }

        return new Matrix($width, $height, $values);
    }

    public function to_json(): string
    {
        $lines = array();
        $max_length = 0;

        for ($y = 0; $y < $this->height; $y += 1)
        {
            $line = array();

            for ($x = 0; $x < $this->width; $x += 1)
            {
                $value = strval($this->get($x, $y));
                array_push($line, "\"" . $value . "\"");

                $max_length = max($max_length, strlen($value));
            }

            array_push($lines, $line);
        }

        $result = array();

        foreach ($lines as $line)
        {
            $parts = array();

            foreach ($line as $part)
            {
                array_push($parts, str_pad($part, $max_length, " ", STR_PAD_LEFT));
            }

            array_push($result, "[" . implode(", ", $parts) . "]");
        }

        return "[" . implode("," . PHP_EOL, $result) . "]";
    }

    public function get(int $x, int $y): BigInteger
    {
        if ($x < 0 || $x >= $this->width)
        {
            throw new InvalidArgumentException("x is out of bounds");
        }

        if ($y < 0 || $y >= $this->height)
        {
            throw new InvalidArgumentException("y is out of bounds");
        }

        return $this->values[$this->width * $y + $x];
    }

    public function set(int $x, int $y, BigInteger $value): void
    {
        if ($x < 0 || $x >= $this->width)
        {
            throw new InvalidArgumentException("x is out of bounds");
        }

        if ($y < 0 || $y >= $this->height)
        {
            throw new InvalidArgumentException("y is out of bounds");
        }

        $this->hash_value = $this->hash_value->xor($this->values[$this->width * $y + $x]);
        $this->hash_value = $this->hash_value->xor($value);

        $this->values[$this->width * $y + $x] = $value;
    }

    public function transpose(): Matrix
    {
        $matrix = Matrix::zero($this->height, $this->width);

        for ($x = 0; $x < $this->width; $x += 1)
        {
            for ($y = 0; $y < $this->height; $y += 1)
            {
                $matrix->set($y, $x, $this->get($x, $y));
            }
        }

        return $matrix;
    }

    public function submatrix(int $x, int $y): Matrix
    {
        if ($x < 0 || $x >= $this->width)
        {
            throw new InvalidArgumentException("x is out of bounds");
        }

        if ($y < 0 || $y >= $this->height)
        {
            throw new InvalidArgumentException("y is out of bounds");
        }

        $matrix = Matrix::zero($this->width - 1, $this->height - 1);

        $new_x = 0;
        $new_y = 0;

        for ($i = 0; $i < $this->width; $i += 1)
        {
            if ($i === $x)
            {
                continue;
            }

            for ($j = 0; $j < $this->height; $j += 1)
            {
                if ($j === $y)
                {
                    continue;
                }

                $matrix->set($new_x, $new_y, $this->get($i, $j));
                $new_y += 1;
            }

            $new_y = 0;
            $new_x += 1;
        }

        return $matrix;
    }

    public function hash(): string
    {
        return strval($this->hash_value);
    }

    public function equals(mixed $other): bool
    {
        if (!($other instanceof Matrix))
        {
            return false;
        }

        if ($this->width !== $other->width || $this->height !== $other->height)
        {
            return false;
        }

        if (!$this->hash_value->equals($other->hash_value))
        {
            return false;
        }

        for ($i = 0; $i < count($this->values); $i += 1)
        {
            if (!$this->values[$i]->equals($other->values[$i]))
            {
                return false;
            }
        }

        return true;
    }

    public function __clone(): void
    {
        for ($i = 0; $i < count($this->values); $i += 1)
        {
            $this->values[$i] = clone $this->values[$i];
        }
    }

    public function __toString(): string
    {
        return $this->to_json();
    }
}
