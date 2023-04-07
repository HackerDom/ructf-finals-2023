<?php

class Parameters
{
    public readonly int $m;
    public readonly int $n;
    public readonly int $k;

    public readonly IntegerRing $ring;
    public readonly MatrixSpace $space;

    private function __construct(int $m, int $n, int $k, IntegerRing $ring, MatrixSpace $space)
    {
        $this->m = $m;
        $this->n = $n;
        $this->k = $k;

        $this->ring = $ring;
        $this->space = $space;
    }

    public static function generate(int $bits, int $m, int $n, int $k): Parameters
    {
        $factors = array();

        for ($i = 0; $i < $m; $i += 1)
        {
            array_push($factors, Parameters::generate_factor($bits));
        }

        $modulus = mul(...$factors);

        $ring = new IntegerRing($modulus);
        $space = new MatrixSpace($ring, $n, $n);

        return new Parameters($m, $n, $k, $ring, $space);
    }

    private static function generate_factor(int $bits): BigInteger
    {
        return BigInteger::random($bits)->next_prime();
    }
}
