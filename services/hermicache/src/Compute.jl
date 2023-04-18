MAX_DEEP = 100


function compute_recurrence_relation_formula(coefficients, base_values, index, deep=0)
    if deep >= MAX_DEEP
        return nothing
    end

    if haskey(base_values, index)
        return base_values[index]
    end

    res = 0
    for i in 1:(length(coefficients) - 1)
        coef_res = compute_recurrence_relation_formula(coefficients, base_values, index - i, deep + 1)
        if coef_res == nothing
            return nothing
        end
        res += coefficients[i] * coef_res
    end

    res += coefficients[length(coefficients)]

    return res
end


function get_longest_palindrome(s, i=1, j=length(s))
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
