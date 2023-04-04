<?php

class MatrixSpace
{
    public readonly int $width;
    public readonly int $height;

    public readonly IntegerRing $ring;

    private MultiKeyCache $cached_cofactors;
    private MultiKeyCache $cached_inversions;
    private MultiKeyCache $cached_submatrices;
    private MultiKeyCache $cached_determinants;

    public function __construct(IntegerRing $ring, int $width, int $height)
    {
        if ($width <= 0 || $height <= 0)
        {
            throw new InvalidArgumentException("width and height must be positive");
        }

        $this->ring = $ring;

        $this->width = $width;
        $this->height = $height;

        $this->cached_cofactors = new MultiKeyCache();
        $this->cached_inversions = new MultiKeyCache();
        $this->cached_submatrices = new MultiKeyCache();
        $this->cached_determinants = new MultiKeyCache();
    }

    public function contains(Matrix $matrix): bool
    {
        return $matrix->width === $this->width && $matrix->height === $this->height;
    }

    public function reduce(Matrix $matrix): Matrix
    {
        if (!$this->contains($matrix))
        {
            throw new InvalidArgumentException("space does not contain matrix");
        }

        $matrix = clone $matrix;

        for ($x = 0; $x < $this->width; $x += 1)
        {
            for ($y = 0; $y < $this->height; $y += 1)
            {
                $value = $matrix->get($x, $y);
                $matrix->set($x, $y, $this->ring->reduce($value));
            }
        }

        return $matrix;
    }

    public function identity(): Matrix
    {
        $values = array_pad(array(), $this->width, BigInteger::one());

        return Matrix::diagonal($values);
    }

    public function determinant(Matrix $matrix): BigInteger
    {
        if (!$this->contains($matrix))
        {
            throw new InvalidArgumentException("space does not contain matrix");
        }

        if ($this->width !== $this->height)
        {
            throw new InvalidArgumentException("matrix must be square");
        }

        return $this->recursive_determinant($matrix);
    }

    private function recursive_determinant(Matrix $matrix): BigInteger
    {
        if ($matrix->width === 1 && $matrix->height === 1)
        {
            return $matrix->get(0, 0);
        }

        $cached_determinant = $this->cached_determinants->get($matrix);

        if ($cached_determinant !== null)
        {
            return $cached_determinant;
        }

        $sum = BigInteger::zero();

        for ($i = 0; $i < $matrix->width; $i += 1)
        {
            $value = $matrix->get($i, 0);

            if ($i % 2 === 0)
            {
                $value = $this->ring->neg($value);
            }

            $submatrix = $this->cached_submatrix($matrix, $i, 0);
            $minor = $this->recursive_determinant($submatrix);

            $sum = $this->ring->add($sum, $this->ring->mul($value, $minor));
        }

        $this->cached_determinants->set($sum, $matrix);

        return $sum;
    }

    public function neg(Matrix $matrix): Matrix
    {
        if (!$this->contains($matrix))
        {
            throw new InvalidArgumentException("space does not contain matrix");
        }

        $matrix = clone $matrix;

        for ($x = 0; $x < $this->width; $x += 1)
        {
            for ($y = 0; $y < $this->height; $y += 1)
            {
                $value = $matrix->get($x, $y);
                $matrix->set($x, $y, $this->ring->neg($value));
            }
        }

        return $matrix;
    }

    public function add(Matrix $left, BigInteger|Matrix $right): Matrix
    {
        if (!$this->contains($left))
        {
            throw new InvalidArgumentException("space does not contain matrix");
        }

        if ($right instanceof Matrix)
        {
            return $this->add_matrix($left, $right);
        }

        return $this->add_integer($left, $right);
    }

    private function add_matrix(Matrix $left, Matrix $right): Matrix
    {
        if ($left->width !== $right->width || $left->height !== $right->height)
        {
            throw new InvalidArgumentException("matrices have different dimensions");
        }

        $matrix = Matrix::zero($this->width, $this->height);

        for ($x = 0; $x < $this->width; $x += 1)
        {
            for ($y = 0; $y < $this->height; $y += 1)
            {
                $value = $this->ring->add($left->get($x, $y), $right->get($x, $y));
                $matrix->set($x, $y, $value);
            }
        }

        return $matrix;
    }

    private function add_integer(Matrix $left, BigInteger $right): Matrix
    {
        $matrix = Matrix::zero($this->width, $this->height);

        for ($x = 0; $x < $this->width; $x += 1)
        {
            for ($y = 0; $y < $this->height; $y += 1)
            {
                $value = $this->ring->add($left->get($x, $y), $right);
                $matrix->set($x, $y, $value);
            }
        }

        return $matrix;
    }

    public function sub(Matrix $left, BigInteger|Matrix $right): Matrix
    {
        if (!$this->contains($left))
        {
            throw new InvalidArgumentException("space does not contain matrix");
        }

        if ($right instanceof Matrix)
        {
            return $this->add($left, $this->neg($right));
        }

        return $this->add($left, $this->ring->neg($right));
    }

    public function mul(Matrix $left, BigInteger|Matrix $right): Matrix
    {
        if ($right instanceof Matrix)
        {
            return $this->mul_matrix($left, $right);
        }

        return $this->mul_integer($left, $right);
    }

    private function mul_matrix(Matrix $left, Matrix $right): Matrix
    {
        if ($left->width !== $this->width || $right->height !== $this->height)
        {
            throw new InvalidArgumentException("space does not contain resulting matrix");
        }

        if ($left->height !== $right->width)
        {
            throw new InvalidArgumentException("invalid matrix dimensions");
        }

        $matrix = Matrix::zero($left->width, $right->height);

        for ($x = 0; $x < $matrix->width; $x += 1)
        {
            for ($y = 0; $y < $matrix->height; $y += 1)
            {
                $sum = BigInteger::zero();

                for ($i = 0; $i < $left->height; $i += 1)
                {
                    $value = $this->ring->mul($left->get($x, $i), $right->get($i, $y));
                    $sum = $this->ring->add($sum, $value);
                }

                $matrix->set($x, $y, $sum);
            }
        }

        return $matrix;
    }

    private function mul_integer(Matrix $left, BigInteger $right): Matrix
    {
        if (!$this->contains($left))
        {
            throw new InvalidArgumentException("space does not contain matrix");
        }

        $matrix = Matrix::zero($this->width, $this->height);

        for ($x = 0; $x < $this->width; $x += 1)
        {
            for ($y = 0; $y < $this->height; $y += 1)
            {
                $value = $this->ring->mul($left->get($x, $y), $right);
                $matrix->set($x, $y, $value);
            }
        }

        return $matrix;
    }

    public function div(Matrix $left, BigInteger|Matrix $right): Matrix
    {
        if ($right instanceof Matrix)
        {
            return $this->mul($left, $this->invert($right));
        }

        return $this->mul($left, $this->ring->invert($right));
    }

    public function cofactor(Matrix $matrix): Matrix
    {
        if (!$this->contains($matrix))
        {
            throw new InvalidArgumentException("space does not contain matrix");
        }

        if ($this->width !== $this->height)
        {
            throw new InvalidArgumentException("matrix must be square");
        }

        $cached_cofactor = $this->cached_cofactors->get($matrix);

        if ($cached_cofactor !== null)
        {
            return $cached_cofactor;
        }

        $result = Matrix::zero($this->width, $this->height);

        for ($x = 0; $x < $result->width; $x += 1)
        {
            for ($y = 0; $y < $result->height; $y += 1)
            {
                $submatrix = $this->cached_submatrix($matrix, $x, $y);
                $minor = $this->recursive_determinant($submatrix);

                if (($x + $y) % 2 == 1) {
                    $minor = $this->ring->neg($minor);
                }

                $result->set($x, $y, $minor);
            }
        }

        $this->cached_cofactors->set($result, $matrix);

        return $result;
    }

    public function invert(Matrix $matrix): Matrix
    {
        if (!$this->contains($matrix))
        {
            throw new InvalidArgumentException("space does not contain matrix");
        }

        if ($this->width !== $this->height)
        {
            throw new InvalidArgumentException("matrix must be square");
        }

        $cached_inversion = $this->cached_inversions->get($matrix);

        if ($cached_inversion !== null)
        {
            return $cached_inversion;
        }

        $result = $this->div(
            $this->cofactor($matrix)->transpose(), $this->determinant($matrix),
        );

        $this->cached_inversions->set($result, $matrix);

        return $result;
    }

    public function pow(Matrix $matrix, BigInteger $exponent): Matrix
    {
        if (!$this->contains($matrix))
        {
            throw new InvalidArgumentException("space does not contain matrix");
        }

        if ($exponent->sign() < 0)
        {
            throw new InvalidArgumentException("invalid exponent");
        }

        $current = clone $matrix;
        $accumulator = $this->identity();

        for ($i = 0; $i < $exponent->bit_size(); $i += 1)
        {
            if ($exponent->hasbit($i))
            {
                $accumulator = $this->mul($accumulator, $current);
            }

            $current = $this->mul($current, $current);
        }

        return $accumulator;
    }

    public function random_element(): Matrix
    {
        $matrix = Matrix::zero($this->width, $this->height);

        for ($x = 0; $x < $this->width; $x += 1)
        {
            for ($y = 0; $y < $this->height; $y += 1)
            {
                $matrix->set($x, $y, $this->ring->random_element());
            }
        }

        return $matrix;
    }

    public function clear_caches(): void
    {
        $this->cached_cofactors->clear();
        $this->cached_inversions->clear();
        $this->cached_submatrices->clear();
        $this->cached_determinants->clear();
    }

    private function cached_submatrix(Matrix $matrix, int $x, int $y): Matrix
    {
        $px = MixedProxy::create($x);
        $py = MixedProxy::create($y);

        $cached_submatrix = $this->cached_submatrices->get($matrix, $px, $py);

        if ($cached_submatrix !== null)
        {
            return $cached_submatrix;
        }

        $result = $matrix->submatrix($x, $y);

        $this->cached_submatrices->set($result, $matrix, $px, $py);

        return $result;
    }

    public function __clone(): void
    {
        $this->ring = clone $this->ring;
    }

    public function __toString(): string
    {
        return "Space of " . $this->width . " by " . $this->height . " matrices over " . $this->ring;
    }
}
