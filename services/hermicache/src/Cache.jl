using CodeTracking
using DataStructures
using Redis


function get_name_to_symbols()
    res = Dict()
    for name in names(@__MODULE__)
        res[string(name)] = name
    end
    return res
end


cache_rc = Redis.RedisConnection(host="localhost", port=6379, password="", db=2)
cache_stat_rc = Redis.RedisConnection(host="localhost", port=6379, password="", db=3)


EXCLUDE = Set()


function get_using_cache_lines(func_name::String, arg_names)
    return [
        "cache_args = [\"$(isempty(arg_names) ? "" : "\$($(join(arg_names, ")\", \"\$(")))\"]")",
        "if any(map((x) -> length(x) > 50, cache_args))",
        "    push!(EXCLUDE, \"$(func_name)\")",
        "end",
        "if \"$(func_name)\" in EXCLUDE",
        "    return _inner(" * join(arg_names, ", ") * ")",
        "end",
#         "cache_key = \"$(func_name)_$(isempty(arg_names) ? "" : "\$($(join(arg_names, ")_\$(")))")\"",
        "cache_key = \"$(func_name)_\" * join(cache_args, \"_\")",
        "cache_key = length(cache_key) > 50 ? cache_key[1:50] : cache_key",
        "r_value = Redis.get(cache_rc, cache_key)",
        "if r_value != nothing",
        "    return r_value",
        "end",
    ]
end


function get_filling_cache_lines(arg_names)
    return [
        "res = _inner(" * join(arg_names, ", ") * ")",
        "stat_value_raw = Redis.get(cache_stat_rc, cache_key)",
        "new_stat_value = stat_value_raw == nothing ? 1 : parse(Int64, stat_value_raw) + 1",
        "Redis.set(cache_stat_rc, cache_key, new_stat_value, \"EX\", 60)",
        "if new_stat_value >= 3",
        "    Redis.set(cache_rc, cache_key, \"EX\", 60)",
        "end",
    ]
end


function get_return_lines()
    return [
        "    return res",
        "end"
    ]
end


function render_lines(lines, ident_count)
    return join(map((x) -> " " ^ ident_count * x, lines), "\n")
end


function patch_function(func)
    println(func)
    evaled_func = eval(func)
    func_methods = methods(evaled_func)

    if length(func_methods) != 1
        return
    end

    func_method = func_methods[1]

    arg_names = Base.method_argnames(func_method)
    arg_names = map(String, arg_names[2:length(arg_names)])

    using_cache_lines = get_using_cache_lines(string(func), arg_names)

    sig_field = getfield(func_method.sig, 3)
    sig_field = Tuple(sig_field[2:length(sig_field)])
    println(sig_field)

    lines = split(code_string(eval(func), sig_field), "\n")

    signature_lines = [lines[1]]
    inner_body = lines[2:length(lines)]

    inner_signature = "function _inner(" * join(arg_names, ", ") * ")"
    inner_definition_lines = [[inner_signature]; inner_body]

    filling_cache_lines = get_filling_cache_lines(arg_names)
    return_lines = get_return_lines()

    main_definition = [
        render_lines(signature_lines, 0);
        render_lines(inner_definition_lines, 8);
        render_lines(using_cache_lines, 4);
        render_lines(filling_cache_lines, 4);
        render_lines(return_lines, 0)
    ]

    code = join(main_definition, "\n")
    println(code)
    eval(Meta.parse(code))
end


function patch_function(func::String, name_to_symbols)
    return patch_function(name_to_symbols[func])
end


function patch_function(func::String)
    return patch_function(func, get_name_to_symbols())
end


function init_cache(m_name)
    for func in names(m_name)
        func_name = string(func)

        try
            evaled_func = eval(func)
            is_func = isa(evaled_func, Function)
            if is_func
                println(func)
#                 patch_function(func_name)
            end
        catch e
            println(e)
        end
    end
end
