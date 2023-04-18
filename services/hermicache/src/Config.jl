struct Config
    redis_host::String
    host::String
    port::Int32
end


prod_config = Config("redis", "0.0.0.0", 8080)
dev_config = Config("localhost", "127.0.0.1", 8080)


function get_config()
    if !haskey(ENV, "ENV")
        throw("Env variable 'ENV' must be defined ('PROD' or 'DEV')")
    end

    if ENV["ENV"] == "PROD"
        return prod_config
    elseif ENV["ENV"] == "DEV"
        return dev_config
    else
        throw("Unknown env type '" * ENV["ENV"] * "'")
    end
end
