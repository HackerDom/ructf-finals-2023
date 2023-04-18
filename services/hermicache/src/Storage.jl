using Base64
using Redis
using UUIDs
using JSON


function is_exists(conn::Redis.RedisConnection, key)
    return Redis.get(conn, key) != nothing
end


function get_user_key(username)
    return joinpath("user", Base64.base64encode(username))
end


function get_field_key(uuid)
    return joinpath("field", string(uuid))
end


function is_user_exists(conn::Redis.RedisConnection, username)
    return is_exists(conn, get_user_key(username))
end


function create_user(conn::Redis.RedisConnection, username, password)
    password_hash_64 = Base64.base64encode(sha256(password))

    Redis.set(conn, get_user_key(username), password_hash_64)
end


function is_equal_base64(password, expected_password_hash_64)
    return expected_password_hash_64 == Base64.base64encode(sha256(password))
end


function is_valid_user_pair(conn::Redis.RedisConnection, username, password)
    expected_password_hash_64 = Redis.get(conn, get_user_key(username))
    if expected_password_hash_64 == nothing
        return false
    end

    return is_equal_base64(password, expected_password_hash_64)
end


function create_field(conn::Redis.RedisConnection, field_record)
    field_uuid = UUIDs.uuid4().value
    Redis.set(conn, get_field_key(field_uuid), JSON.json(field_record))
    Redis.rpush(conn, joinpath("username2fields", field_record["owner"]), field_uuid)

    return field_uuid
end


function get_field(conn::Redis.RedisConnection, uuid)
    raw_field = Redis.get(conn, get_field_key(uuid))
    return raw_field == nothing ? raw_field : JSON.parse(raw_field)
end


function list_fields(conn::Redis.RedisConnection, owner)
    res = Redis.lrange(conn, joinpath("username2fields", owner), 0, -1)
    return res == nothing ? String[] : res
end
