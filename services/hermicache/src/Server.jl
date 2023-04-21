using Bukdu
using HTTP
using Redis
using JSONSchema
using DataStructures


include("Cache.jl")
include("Compute.jl")
include("Config.jl")
include("RedisCommands.jl")
include("Storage.jl")
include("SessionManager.jl")


struct RestController <: ApplicationController
    conn::Conn
end


config = get_config()


session_rc = Redis.RedisConnection(host=config.redis_host, port=6379, password="", db=0)
storage_rc = Redis.RedisConnection(host=config.redis_host, port=6379, password="", db=0)


function code_and_message(c::RestController, code, text)
    c.conn.request.response.status = code
    return render(Text, text)
end


function add_header(c::RestController, header, value)
    push!(c.conn.request.response.headers, Pair(header, value))
end


function add_cookie(c::RestController, key, value)
    add_header(c, "Set-Cookie", key * "=" * value)
end


function register_handler(c::RestController, message)
    username = message["username"]
    password = message["password"]

    if is_user_exists(storage_rc, username)
        return code_and_message(c, 400, "user is already exists")
    end

    create_user(storage_rc, username, password)
    secret = create_session(session_rc, username)

    add_cookie(c, "secret", secret)
    add_cookie(c, "username", username)
    render(Text, "OK")
end


function login_handler(c::RestController, message)
    username = message["username"]
    password = message["password"]

    if !is_valid_user_pair(storage_rc, username, password)
        return code_and_message(c, 400, "invalid username or password")
    end

    secret = create_session(session_rc, username)
    add_cookie(c, "secret", secret)
    add_cookie(c, "username", username)
    render(Text, "OK")
end


function get_validator(name)
    open(joinpath("schemas", name)) do f
        raw_validator = read(f, String)
        validator = JSONSchema.Schema(raw_validator)

        function validator_func(message)
            return Base.isvalid(validator, message)
        end

        return validator_func
    end
end


function with_body_validator(handler, validator)
    function wrapped_handler(c::RestController)
        try
            message = JSON.parse(String(c.conn.request.body))
            if !validator(message)
                return code_and_message(c, 400, "invalid message schema")
            end

            try
                return handler(c, message)
            catch handler_processing_e
                Plug.Loggers.print_message("Error: ", handler_processing_e)
                Base.show_backtrace(stderr, backtrace())
                return code_and_message(c, 500, "error while processing handler")
            end
        catch unhandled_error
            if unhandled_error isa ErrorException
                c.conn.request.response.status = 400
                return render(Text, "invalid json")
            end

            Plug.Loggers.print_message("Unhandled error: ", unhandled_error)
            Base.show_backtrace(stderr, backtrace())
            return code_and_message(c, 500, "unhandled error")
        end
    end

    return wrapped_handler
end


function with_query_params_validator(handler, required_params)
    function wrapped_handler(c::RestController)

    end
end


# with_validator: f(c, message) -> f(c)
# with_auth1: f(c, username, message) -> f(c, message)


function with_auth(handler)
    function wrapped_handler(c::RestController, args...)
        secret_cookies = HTTP.Cookies.readcookies(c.conn.request.headers, "secret")
        username_cookies = HTTP.Cookies.readcookies(c.conn.request.headers, "username")
        if length(secret_cookies) != 1 || length(username_cookies) != 1
            return code_and_message(c, 401, "unauthorized")
        end

        secret = secret_cookies[1].value
        username = username_cookies[1].value

        if !validate_session(session_rc, username, secret)
            return code_and_message(c, 401, "unauthorized")
        end

        return handler(c, username, args...)
    end

    return wrapped_handler
end


function create_field_handler(c::RestController, username, message)
    field_record = Dict(
        "type"=>message["type"],
        "content"=>message["content"],
        "owner"=>username
    )

    field_uuid = create_field(storage_rc, field_record)

    res = Dict(
        "uuid"=>field_uuid
    )
    return render(JSON, res)
end


function with_required_params(handler, required_params)
    function wrapped_handler(c::RestController)
        param_dict = HTTP.queryparams(HTTP.URI(c.conn.request.target))
        for required_param in required_params
            if !haskey(param_dict, required_param)
                return code_and_message(c, 400, "invalid params")
            end
        end

        try
           return handler(c, param_dict)
        catch handler_processing_e
            Plug.Loggers.print_message("Error: ", handler_processing_e)
            Base.show_backtrace(stderr, backtrace())
            return code_and_message(c, 500, "error while processing handler")
        end
    end

    return wrapped_handler
end


function is_valid_uuid(raw_uuid)
    try
        parse(UInt128, raw_uuid)
        return true
    catch e
        return false
    end
end


function get_field_handler(c::RestController, username)
    field_uuid = c.params.field_uuid
    if !is_valid_uuid(field_uuid)
        return code_and_message(c, 400, "invalid uuid")
    end

    field = get_field(storage_rc, field_uuid)
    if field == nothing || field["owner"] != username
        return code_and_message(c, 404, "field not found")
    end

    return render(JSON, field)
end


function compute_handler(c::RestController, username, param_dict)
    field_uuid = param_dict["field_uuid"]
    arg = param_dict["arg"]

    if !is_valid_uuid(field_uuid)
        return code_and_message(c, 400, "invalid uuid")
    end

    field = get_field(storage_rc, field_uuid)
    if field == nothing || field["owner"] != username
        return code_and_message(c, 404, "field not found")
    end

    if field["type"] == "palindrome"
        res = get_longest_palindrome(field["content"], 1, length(field["content"]))
        return render(JSON, Dict(
            "res"=>res
        ))
    end

    if field["type"] == "recurrent"
        base_dict = Dict()
        base_pairs = field["content"]["base"]
        for pair_index in 1:length(base_pairs)
            base_dict[base_pairs[pair_index][1]] = base_pairs[pair_index][2]
        end

        try
            index = parse(Int64, arg)
            res = compute_rec(field["content"]["coefficients"], base_dict, index, 0)
            if res == nothing
                return code_and_message(c, 400, "recursion limit exceeded")
            end

            return render(JSON, Dict(
                "res"=>res
            ))
        catch e
            return code_and_message(c, 400, "invalid arg")
        end
    end
end


function list_fields_handler(c::RestController, username)
    fields = list_fields(storage_rc, username)
    return render(JSON, fields)
end


routes() do
    post("/register/", RestController, with_body_validator(register_handler, get_validator("user_pair.json")))
    post("/login/", RestController, with_body_validator(login_handler, get_validator("user_pair.json")))

    post("/field/", RestController, with_body_validator(with_auth(create_field_handler), get_validator("create_field.json")))
    get("/field/:field_uuid/", RestController, with_auth(get_field_handler))
    get("/compute/", RestController, with_required_params(with_auth(compute_handler), ["field_uuid", "arg"]))

    get("/list_fields/", RestController, with_auth(list_fields_handler))

    plug(Plug.Parsers, [:json])
end


init_cache(@__MODULE__)

Bukdu.start(config.port, host=config.host)
Base.JLOptions().isinteractive == 0 && wait()
