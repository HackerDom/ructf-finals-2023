using Redis
using Random
using SHA
using Base64


function get_secret(username, salt)
    return Base64.base64encode(sha256(username * salt))
end


function create_session(conn::Redis.RedisConnection, username)
    salt = randstring(20)
    Redis.set(conn, username, salt)
    return get_secret(username, salt)
end


function delete_session(conn::Redis.RedisConnection, username)
    return Redis.del(conn, username)
end


function validate_session(conn::Redis.RedisConnection, username, secret)
    salt = Redis.get(conn, username)
    if salt == nothing
        return false
    end
    return Base64.base64encode(sha256(username * salt)) == secret
end
