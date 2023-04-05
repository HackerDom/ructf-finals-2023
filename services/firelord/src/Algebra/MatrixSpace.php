<?php

class MatrixSpace
{
    public readonly int $width;
    public readonly int $height;

    public readonly IntegerRing $ring;

    private MultiKeyCache $cached_adjugates;
    private MultiKeyCache $cached_submatrices;
    private MultiKeyCache $cached_determinants;

    public function __construct(IntegerRing $ring, int $width, int $height)
    {
        if ($width <= 0 || $height <= 0)
        {
            throw new InvalidArgumentException("MatrixSpace: width and height must be positive");
        }

        $this->ring = $ring;

        $this->width = $width;
        $this->height = $height;

        $this->cached_adjugates = new MultiKeyCache();
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
            throw new InvalidArgumentException("MatrixSpace::reduce: space does not contain matrix");
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
            throw new InvalidArgumentException("MatrixSpace::determinant: space does not contain matrix");
        }

        if (!$matrix->is_square())
        {
            throw new InvalidArgumentException("MatrixSpace::determinant: matrix must be square");
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
            throw new InvalidArgumentException("MatrixSpace::neg: space does not contain matrix");
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
            throw new InvalidArgumentException("MatrixSpace::add: space does not contain left matrix");
        }

        if ($right instanceof Matrix)
        {
            if (!$this->contains($right))
            {
                throw new InvalidArgumentException("MatrixSpace::add: space does not contain right matrix");
            }

            return $this->add_matrix($left, $right);
        }

        return $this->add_integer($left, $right);
    }

    private function add_matrix(Matrix $left, Matrix $right): Matrix
    {
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
            throw new InvalidArgumentException("MatrixSpace::sub: space does not contain left matrix");
        }

        if ($right instanceof Matrix)
        {
            if (!$this->contains($right))
            {
                throw new InvalidArgumentException("MatrixSpace::sub: space does not contain right matrix");
            }

            return $this->add_matrix($left, $this->neg($right));
        }

        return $this->add_integer($left, $this->ring->neg($right));
    }

    public function mul(Matrix $left, BigInteger|Matrix $right): Matrix
    {
        if ($right instanceof Matrix)
        {
            if ($left->height !== $right->width)
            {
                throw new InvalidArgumentException("MatrixSpace::mul: invalid matrix dimensions");
            }

            if ($left->width !== $this->width || $right->height !== $this->height)
            {
                throw new InvalidArgumentException("MatrixSpace::mul: space does not contain resulting matrix");
            }

            return $this->mul_matrix($left, $right);
        }

        if (!$this->contains($left))
        {
            throw new InvalidArgumentException("MatrixSpace::mul: space does not contain left matrix");
        }

        return $this->mul_integer($left, $right);
    }

    private function mul_matrix(Matrix $left, Matrix $right): Matrix
    {
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
            $right_inv = $this->invert($right);

            if ($left->height !== $right_inv->width)
            {
                throw new InvalidArgumentException("MatrixSpace::div: invalid matrix dimensions");
            }

            if ($left->width !== $this->width || $right_inv->height !== $this->height)
            {
                throw new InvalidArgumentException("MatrixSpace::div: space does not contain resulting matrix");
            }

            return $this->mul_matrix($left, $right_inv);
        }

        if (!$this->contains($left))
        {
            throw new InvalidArgumentException("MatrixSpace::div: space does not contain left matrix");
        }

        return $this->mul_integer($left, $this->ring->invert($right));
    }

    public function adjugate(Matrix $matrix): Matrix
    {
        if (!$this->contains($matrix))
        {
            throw new InvalidArgumentException("MatrixSpace::cofactor_matrix: space does not contain matrix");
        }

        if (!$matrix->is_square())
        {
            throw new InvalidArgumentException("MatrixSpace::cofactor_matrix: matrix must be square");
        }

        if ($matrix->width === 1 && $matrix->height === 1)
        {
            return Matrix::diagonal(array(BigInteger::one()));
        }

        $cached_adjugate = $this->cached_adjugates->get($matrix);

        if ($cached_adjugate !== null)
        {
            return $cached_adjugate;
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

        $result = $result->transpose();

        $this->cached_adjugates->set($result, $matrix);

        return $result;
    }

    public function invert(Matrix $matrix): Matrix
    {
        if (!$this->contains($matrix))
        {
            throw new InvalidArgumentException("MatrixSpace::invert: space does not contain matrix");
        }

        if (!$matrix->is_square())
        {
            throw new InvalidArgumentException("MatrixSpace::invert: matrix must be square");
        }

        if (!$this->determinant($matrix))
        {
            throw new InvalidArgumentException("MatrixSpace::invert: matrix is not invertible");
        }

        $result = $this->div($this->adjugate($matrix), $this->determinant($matrix));

        return $result;
    }

    public function pow(Matrix $matrix, BigInteger $exponent): Matrix
    {
        if (!$this->contains($matrix))
        {
            throw new InvalidArgumentException("MatrixSpace::pow: space does not contain matrix");
        }

        if (!$matrix->is_square())
        {
            throw new InvalidArgumentException("MatrixSpace::pow: matrix must be square");
        }

        if ($exponent->sign() < 0)
        {
            throw new InvalidArgumentException("MatrixSpace::pow: invalid exponent");
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
        $this->cached_adjugates->clear();
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
