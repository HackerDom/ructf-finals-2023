package ru.ctf.http;

import java.io.IOException;
import java.net.URI;

import io.netty.handler.codec.http.FullHttpRequest;
import io.netty.handler.codec.http.FullHttpResponse;
import io.netty.handler.codec.http.HttpMethod;

public interface HandlingStrategy {
    HttpMethod method();

    URI uri();

    FullHttpResponse handle(FullHttpRequest request) throws IOException;
}
