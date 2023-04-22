# Hermicache

## Description
Hermicache is a simple service for solving one of followed problems: [Recurrent Formula Computation](https://en.wikipedia.org/wiki/Recurrence_relation) or [Longest Palindromic Subsequence](https://leetcode.com/problems/longest-palindromic-subsequence/) written in Julia. 

Allowed methods:
* POST `/register` - register a new user by username and password (returns cookies in response), example: `curl -X POST "http://localhost/register" -d '{"username": "username", "password": "12345678"}'`;
* POST `/login` - login to an existing account (returns cookies in response), example: `curl -X POST "http://localhost/login" -d '{"username": "username", "password": "12345678"}'`;
* POST `/field` - create a new field, example (wo cookies): `curl -X POST "http://localhost/field" -d '{"type": "palindrome", "content": "ABCHFOPFCHBTAQ"}'` (returns uuid);
* GET `/field/{uuid}` - get field by uuid (require auth via cookies) and returns ticket id, example: `curl "http://localhost/field/39373405466312737284093885958213795484"`;
* GET `/list_fields` - get field uuid list (require auth via cookies);
* GET `/compute?field_uuid=<field_uuid>&arg=<arg>` - make a compute request with passed field uuid and argument (require auth via cookies) ;


### Field
Field is a data which allows you to make a compute request. It can be coefficients and base values for the Recurrent Formula Computation or string for the Longest Palindromic Subsequence.
You can find full schema definition here: https://github.com/HackerDom/ructf-finals-2023/blob/master/services/hermicache/src/schemas/create_field.json
Example for palindrome problem: content is just a string: `{"type": "palindrome", "content": "ABCHFOPFCHBTAQ"}`
Example for recurrent formula for the [fibonacci sequence](https://en.wikipedia.org/wiki/Fibonacci_sequence):
`{'type': 'recurrent', 'content': {'base': [[0, 1], [1, 1]], 'coefficients': [1, 1, 0]}}`
``` 
// fibonacci sequence 
f(0) := 1  // base values
f(1) := 1
f(n) := 1 * f(n - 1) + 1 * f(n - 2) + 0  // coefficients 1, 1, 1
```

### Caching
Recurrent Formula Computation and Longest Palindromic Subsequence - both of problems has a recursive [implementation](https://github.com/HackerDom/ructf-finals-2023/blob/master/services/hermicache/src/Compute.jl).
Moreover, these functions are pure. So we can cache every call to avoid excess computing function with the same arguments.
Caching in hermicache is [implemented](https://github.com/HackerDom/ructf-finals-2023/blob/master/services/hermicache/src/Cache.jl#L115) **semi-global**. It means that in `init_cache` call program will find all functions and try to cache it.
However, some functions will be added to EXCLUDE set due to too long arguments just in runtime. All functions from EXCLUDE set will not use the cache.
Functions will be cached only after 3 calls and only for 60 seconds for avoiding large memory using. 

## Vulnerability
Caching is written with the caching key generation as stringized argument [concatenation](https://github.com/HackerDom/ructf-finals-2023/blob/master/services/hermicache/src/Cache.jl#L30) (also with function name)
Moreover, caching key is slicing up by length 90 chars. It all leads to caching concept violation:
caching can generate same caching key for the different arguments. 

So if we trigger cache for the function [is_equal_base64](https://github.com/HackerDom/ructf-finals-2023/blob/master/services/hermicache/src/Storage.jl#L33),
which tests the (password / base64ed password hash) pair correctness, we can get cached value for some different arguments.
Fortunately, we have this argument order: `password`, `expected_password_hash_64`. Caching key will be equaled `is_equal_base64_<password>_<expected_password_hash_64>`.
So if the `is_equal_base64_<password>_` part will have length 90 chars, function will be cached only by the passed password.

With that knowing we can build this call sequence (password length must be at least `90 - len('is_equal_base64_') + len('_') = 73`. So total caching key length will be at least 90):
```
is_equal_base64('LONG_PASSWORD_...', <some_hash>)  // call
is_equal_base64('LONG_PASSWORD_...', <some_hash>)  // call
is_equal_base64('LONG_PASSWORD_...', <some_hash>)  // third call is caching

is_equal_base64('LONG_PASSWORD...', <some_another_hash>)  // call with another base64ed_hash will return same value!
```

## Attack
Knowing the cache vulnerability we can forge cookies in that way:

* register a user with password, len(password) = 73;
* login with same username / password 3 times. It will cache function with value `true` with caching key defined only by password; 
* login with any other username, but same password with length = 73, cached value `true` will be returned. Now we have a forged cookies;
* list field uuids and get all content of fields with forged cookies.

## Defense
You can not just remove `init_cache` because it dramatically slows down both of problems.
The first way is rewrite both of algorithms with explicit caching and disable semi-global caching.
Another way is fix semi-global caching.
