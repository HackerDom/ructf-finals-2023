using Redis


reentrant_lock = ReentrantLock()


expiration_time = 60 * 20

function Redis_set(conn, key, value)
    lock(reentrant_lock) do
        Redis.set(conn, key, value, "EX", expiration_time)
    end
end

function Redis_get(conn, key)
    lock(reentrant_lock) do
        return Redis.get(conn, key)
    end
end

function Redis_rpush(conn, key, value)
    lock(reentrant_lock) do
        Redis.rpush(conn, key, value)
        Redis.expire(conn, key, expiration_time)
    end
end

function Redis_lrange(conn, key, l, r)
    lock(reentrant_lock) do
        Redis.lrange(conn, key, l, r)
    end
end
