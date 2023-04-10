# RuCTF Finals 2023 | solaris

## Description

The service was named after [Solaris](https://en.wikipedia.org/wiki/Solaris_(1972_film)) — a 1972 Soviet science fiction movie directed by [Andrei Tarkovsky](https://en.wikipedia.org/wiki/Andrei_Tarkovsky).

## Cryptosystem

Prepare parameters:

1. Generate an RSA modulus `N = p * q` (where `p` and `q` are prime integers). Integers modulo `N` form ring `R`

2. Declare `MS` — space of all matrices `n x n` over the ring `R`

3. Select some random invertible matrices `A = {A1, A2, ..., Ak}` from `MS`

4. Use `N` as a public key and `A` as a private key

Declare operation `MASK(x)`:

1. Select `r = (r1, r2, ..., r{n-1})` as random elements from `R`

2. Create diagonal matrix `M` using vector `(m, ...r)`

3. Return `M`

Declare operation `UNMASK(X)`:

1. Return top-left element of matrix `X`

Let's say we have to encrypt a message `m`. Encryption and decryption operations:

```
Enc(m) = A^-1 * MASK(m) * A = 
     = Ak^-1 * ... * A2^-1 * A1^-1 * MASK(m) * A1 * A2 * ... * Ak

Dec(M) = UNMASK(A * M * A^-1) = 
     = UNMASK(A1 * A2 * ... * Ak * M * Ak^-1 * ... * A2^-1 * A1^-1)
```

## Vulnerability

Suppose we want to decrypt ciphertext `M` and recover plaintext `m`.

1. Take a [trace](https://en.wikipedia.org/wiki/Trace_(linear_algebra)) of matrix `M` as a sum of main diagonal using [similarity invariance](https://en.wikipedia.org/wiki/Similarity_invariance)

```
t1 = trace(M) = m + r1 + r2 + ... + r{n-1}
```

We have a single equation of `n` variables, where `n` is the dimension of the matrix. Since we have `n` variables, we need at least `n` equations to solve the system.

2. Take traces of matrices `M^2, M^3, ..., M^n`, where `^` is a exponentiation operation

```
t2 = trace(M^2) = m^2 + r1^2 + r2^2 + ... + r{n_1}^2
t3 = trace(M^3) = m^3 + r1^3 + r2^3 + ... + r{n_1}^3
...
tn = trace(M^n) = m^n + r1^n + r2^n + ... + r{n_1}^n
```

Now we have the system of `n` variables and `n` equations. Let's look at the polynomial form:

```
pol_1(X, Y1, Y2, ..., Y{n-1}) = X^1 + Y1^1 + Y2^1 + ... + Y{n-1}^1 - t1
pol_2(X, Y1, Y2, ..., Y{n-1}) = X^2 + Y1^2 + Y2^2 + ... + Y{n-1}^2 - t2
...
pol_n(X, Y1, Y2, ..., Y{n-1}) = X^n + Y1^n + Y2^n + ... + Y{n-1}^n - tn
```

3. Calculate the [Gröbner basis](https://en.wikipedia.org/wiki/Gr%C3%B6bner_basis) to get univariate polynomial

The calculated basis contains a univariate monic polynomial `P(W)` over the ring `R`:

```
P(W) = W^n + U{n-1}*W^(n-1) + U{n-2}*W^(n-2) + ... + U2*W^2 + U1*W + U0
```

This polynomial contains all solutions for the system above:

```
P(m) = 0
P(r1) = 0
P(r2) = 0
...
P(r{n-1}) = 0
```

We can't directly find roots of `P(W)`, because we don't know factorization of `N`.

4. Apply [Coppersmith method](https://en.wikipedia.org/wiki/Coppersmith_method) to get small roots

Service used:

- `N ~ 2048 bits`
- `n = 6`
- `m ~ 256 bits` (RuCTF flag format)

Degree of `P(W)` is `n`, so `m^n ~ 256*6 ~ 1536 bits`.

Fortunately `m^n < N` so we can obtain `m` as a small root of `P(W)`.

Example exploit: [sploit.sage](/sploits/solaris/sploit.sage)

## Patch

Just change `bits` from 1024 to 512. Then `N ~ 1024 bits` and `m^n > N`.

Example patch: [patch.diff](patch.diff)
