<?php

include_once "autoload.php";

function test_matrix_transpose(): void
{
    $R = new IntegerRing(BigInteger::random(512));

    for ($size = 1; $size < 10; $size += 1)
    {
        $MS = new MatrixSpace($R, $size, $size);

        foreach (range(0, 10) as $test)
        {
            $m1 = $MS->random_element();
            $m2 = $m1->transpose()->transpose();

            assert($m2->equals($m1));
        }
    }
}

function test_matrix_serialization(): void
{
    $R = new IntegerRing(BigInteger::random(512));

    for ($size = 1; $size < 10; $size += 1)
    {
        $MS = new MatrixSpace($R, $size, $size);

        foreach (range(0, 10) as $test)
        {
            $m1 = $MS->random_element();
            $m2 = Matrix::from_json(Matrix::from_json($m1->to_json())->to_json());

            assert($m2->equals($m1));
        }
    }
}

function test_integer_ring(): void
{
    $modulus = BigInteger::random(512)->next_prime();
    $R = new IntegerRing($modulus);

    foreach (range(0, 10) as $test)
    {
        $n1 = $R->random_element();
        $n2 = $R->neg($R->neg($n1));

        assert($n2->equals($n1));
    }

    foreach (range(0, 10) as $test)
    {
        $n1 = $R->random_element();
        $n2 = $R->invert($R->invert($n1));

        assert($n2->equals($n1));
    }

    foreach (range(0, 10) as $test)
    {
        $n1 = $R->random_element();
        $n2 = $R->random_element();
        $n3 = $R->sub($R->add($n1, $n2), $n2);

        assert($n3->equals($n1));
    }

    foreach (range(0, 10) as $test)
    {
        $n1 = $R->random_element();
        $n2 = $R->random_element();
        $n3 = $R->div($R->mul($n1, $n2), $n2);

        assert($n3->equals($n1));
    }

    foreach (range(0, 10) as $test)
    {
        $n1 = $R->random_element();
        $n2 = $R->pow($n1, $modulus->sub(BigInteger::one()));

        assert($n2->equals(BigInteger::one()));
    }
}

function test_matrix_space(): void
{
    $modulus = BigInteger::random(512)->next_prime();
    $R = new IntegerRing($modulus);
    $MS = new MatrixSpace($R, 5, 5);

    foreach (range(0, 10) as $test)
    {
        $m1 = $MS->random_element();
        $m2 = $MS->neg($MS->neg($m1));

        assert($m2->equals($m1));
    }

    foreach (range(0, 10) as $test)
    {
        $m1 = $MS->random_element();
        $m2 = $MS->invert($MS->invert($m1));

        assert($m2->equals($m1));
    }

    foreach (range(0, 10) as $test)
    {
        $m1 = $MS->random_element();
        $m2 = $MS->mul($m1, $MS->invert($m1));

        assert($m2->equals($MS->identity()));
    }

    foreach (range(0, 10) as $test)
    {
        $m1 = $MS->random_element();
        $m2 = $MS->random_element();
        $m3 = $MS->sub($MS->add($m1, $m2), $m2);

        assert($m3->equals($m1));
    }

    foreach (range(0, 10) as $test)
    {
        $n = $R->random_element();
        $m1 = $MS->random_element();
        $m2 = $MS->sub($MS->add($m1, $n), $n);

        assert($m2->equals($m1));
    }

    foreach (range(0, 10) as $test)
    {
        $m1 = $MS->random_element();
        $m2 = $MS->random_element();
        $m3 = $MS->div($MS->mul($m1, $m2), $m2);

        assert($m3->equals($m1));
    }

    foreach (range(0, 10) as $test)
    {
        $n = $R->random_element();
        $m1 = $MS->random_element();
        $m2 = $MS->div($MS->mul($m1, $n), $n);

        assert($m2->equals($m1));
    }

    foreach (range(0, 10) as $test)
    {
        $m1 = $MS->random_element();
        $m2 = $MS->pow($m1, BigInteger::from(3));
        $m3 = $MS->mul($MS->mul($m1, $m1), $m1);

        assert($m2->equals($m3));
    }

    foreach (range(0, 10) as $test)
    {
        $m = $MS->random_element();

        $d1 = $MS->determinant($m);
        $d2 = $MS->determinant($m->transpose());

        assert($d2->equals($d1));
    }
}

function test_mask(): void
{
    $R = new IntegerRing(BigInteger::random(512));

    for ($size = 1; $size < 10; $size += 1)
    {
        foreach (range(0, 10) as $test)
        {
            $array = array();

            for ($i = 0; $i < $size; $i += 1)
            {
                array_push($array, $R->random_element());
            }

            $m1 = Matrix::diagonal($array);
            $m2 = Mask::from_matrix(Mask::from_matrix($m1)->to_matrix())->to_matrix();

            assert($m2->equals($m1));
        }
    }
}

function test_masker(): void
{
    for ($size = 1; $size < 10; $size += 1)
    {
        $parameters = Parameters::generate(1024, 1, $size, 0);
        $key = Key::generate($parameters);

        foreach (range(0, 10) as $test)
        {
            $data1 = str_pad("", 127, "A");
            $data2 = Masker::unmask(
                $key, Masker::mask(
                    $key, Masker::unmask(
                        $key, Masker::mask(
                            $key, $data1,
                        ),
                    ),
                ),
            );

            assert($data2 === $data1);
        }
    }
}

function test_cipher(): void
{
    for ($size = 1; $size < 10; $size += 1)
    {
        $parameters = Parameters::generate(512, 2, $size, 2);
        $key = Key::generate($parameters);

        foreach (range(0, 10) as $test)
        {
            $data1 = str_pad("", 127, "A");
            $data2 = Masker::unmask(
                $key, Cipher::decrypt(
                    $key, Cipher::encrypt(
                        $key, Masker::mask(
                            $key, $data1,
                        ),
                    ),
                ),
            );

            assert($data2 === $data1);
        }
    }
}

function test_padding(): void
{
    for ($source_length = 10; $source_length < 100; $source_length += 10)
    {
        for ($padded_length = 10; $padded_length < 100; $padded_length += 10)
        {
            $data1 = str_pad("", $source_length, "A");
            $data2 = unpad(pad($data1, $padded_length));

            assert($data2 === $data1);
        }
    }
}

function test_cache(): void
{
    $R = new IntegerRing(BigInteger::random(512));
    $MS = new MatrixSpace($R, 5, 5);

    $cache = new MultiKeyCache();

    $bigint = $R->random_element();
    $matrix = $MS->random_element();
    $proxy = MixedProxy::create("agj8fh29fuh9fna");

    $cache->set(true, $bigint);
    $cache->set(true, $matrix);
    $cache->set(true, $proxy);
    $cache->set(true, $bigint, $matrix);
    $cache->set(true, $matrix, $proxy);
    $cache->set(true, $bigint, $matrix, $proxy);

    assert($cache->get($bigint));
    assert($cache->get($matrix));
    assert($cache->get($proxy));
    assert($cache->get($bigint, $matrix));
    assert($cache->get($matrix, $proxy));
    assert($cache->get($bigint, $matrix, $proxy));

    assert($cache->get($matrix, $bigint) === null);
    assert($cache->get($proxy, $bigint) === null);
    assert($cache->get($matrix, $bigint, $proxy) === null);
}

function test_all(): void
{
    test_matrix_transpose();
    test_matrix_serialization();
    test_integer_ring();
    test_matrix_space();
    test_mask();
    test_masker();
    test_cipher();
    test_padding();
    test_cache();
}

test_all();
