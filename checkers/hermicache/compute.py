def get_longest_palindrome(s):
    cache = {}

    def _get_longest_palindrome(s, i, j):
        if (i, j) in cache:
            return cache[i, j]

        if i == j:
            return 1

        if i + 1 == j:
            if s[i] == s[j]:
                return 2
            else:
                return 1

        if s[i] != s[j]:
            res = max(_get_longest_palindrome(s, i, j - 1), _get_longest_palindrome(s, i + 1, j))
            cache[i, j] = res
            return res

        res = _get_longest_palindrome(s, i + 1, j - 1) + 2
        cache[i, j] = res
        return res

    return _get_longest_palindrome(s, 0, len(s) - 1)


def compute_recurrence_relation_formula(coefficients, base_values, index, cache):
    max_deep = 100
    big_modulus = 10 ** 9 + 7

    def _compute_recurrence_relation_formula(coefficients, base_values, index, deep):
        if index in cache:
            return cache[index]
        if deep >= max_deep:
            return None

        if index in base_values:
            return base_values[index]

        res = 0
        for i in range(len(coefficients) - 1):
            coef_res = _compute_recurrence_relation_formula(coefficients, base_values, index - i - 1, deep + 1)
            if coef_res is None:
                return None
            res += coefficients[i] * coef_res
            res %= big_modulus

        res += coefficients[len(coefficients) - 1]
        res %= big_modulus

        cache[index] = res
        return res
    return _compute_recurrence_relation_formula(coefficients, base_values, index, 0)


def compute_recurrence_relation_formula_by_field(field, arg, cache):
    return compute_recurrence_relation_formula(
        field['content']['coefficients'],
        dict(field['content']['base']),
        arg,
        cache
    )
