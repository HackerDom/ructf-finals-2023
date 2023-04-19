MAX_DEEP = 100
BIG_MODULUS = 1000000007

function compute_rec(coefficients, base_values, index, deep)
    res::Int128 = 0

    if deep >= MAX_DEEP
        return nothing
    end

    if haskey(base_values, index)
        return base_values[index]
    end

    for i in 1:(length(coefficients) - 1)
        coef_res = compute_rec(coefficients, base_values, index - i, deep + 1)
        if coef_res == nothing
            return nothing
        end
        res += coefficients[i] * coef_res
        res %= BIG_MODULUS
    end

    res += coefficients[length(coefficients)]
    res %= BIG_MODULUS

    return res > 0 ? res : res + BIG_MODULUS
end


function get_longest_palindrome(s, i, j)
    if i == j
        return 1
    end

    if i + 1 == j
        if s[i] == s[j]
            return 2
        else
            return 1
        end
    end

    if s[i] != s[j]
        return max(get_longest_palindrome(s, i, j - 1), get_longest_palindrome(s, i + 1, j))
    end

    return get_longest_palindrome(s, i + 1, j - 1) + 2
end
