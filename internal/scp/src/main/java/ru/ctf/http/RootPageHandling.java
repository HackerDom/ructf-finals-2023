package ru.ctf.http;

import java.io.IOException;
import java.net.URI;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import com.fasterxml.jackson.databind.ObjectMapper;
import io.netty.buffer.Unpooled;
import io.netty.handler.codec.http.DefaultFullHttpResponse;
import io.netty.handler.codec.http.FullHttpRequest;
import io.netty.handler.codec.http.FullHttpResponse;
import io.netty.handler.codec.http.HttpHeaderNames;
import io.netty.handler.codec.http.HttpHeaderValues;
import io.netty.handler.codec.http.HttpMethod;
import io.netty.handler.codec.http.HttpResponseStatus;
import io.netty.util.CharsetUtil;
import ru.ctf.doc.dao.DAO;
import ru.ctf.http.dto.RootRequest;
import ru.ctf.scpql.SCPQL;
import ru.ctf.user.User;
import ru.ctf.user.dao.UserDAO;

import static io.netty.handler.codec.http.HttpHeaderNames.CONTENT_TYPE;
import static io.netty.handler.codec.http.HttpVersion.HTTP_1_1;

/**
 * SCPQL request handler
 */
public class RootPageHandling {
    private static final URI ROOT = URI.create("/");

    public static class ContentStrategy implements HandlingStrategy {

        @Override
        public HttpMethod method() {
            return HttpMethod.GET;
        }

        @Override
        public URI uri() {
            return ROOT;
        }

        @Override
        public FullHttpResponse handle(FullHttpRequest request) {
            var resp = new DefaultFullHttpResponse(HTTP_1_1, HttpResponseStatus.OK);
            resp.headers().set(CONTENT_TYPE, "text/html; charset=UTF-8");
            try {
                Path path = Paths.get("static/index.html");
                var content = Files.readAllBytes(path);
                resp.content().writeBytes(content);
                return resp;
            } catch (IOException e) {
                return new DefaultFullHttpResponse(HTTP_1_1, HttpResponseStatus.INTERNAL_SERVER_ERROR);
            }
        }
    }

    public static class ActionStrategy implements HandlingStrategy {

        private static final ObjectMapper mapper = new ObjectMapper();
        private static final DAO<String, User> userDao = UserDAO.getInstance();

        @Override
        public HttpMethod method() {
            return HttpMethod.POST;
        }

        @Override
        public URI uri() {
            return ROOT;
        }

        @Override
        public FullHttpResponse handle(FullHttpRequest request) throws IOException {
            byte[] content = new byte[request.content().readableBytes()];
            request.content().readBytes(content);
            try {
                var rootRequest = mapper.readValue(content, RootRequest.class);
                var user = userDao.get(rootRequest.token());
                if (null == user) {
                    return new DefaultFullHttpResponse(HTTP_1_1, HttpResponseStatus.NOT_FOUND);
                }
                var ql = new SCPQL(user);
                ql.process(rootRequest.query());
                var json = mapper.writeValueAsString(ql.getResult());
                var resp = new DefaultFullHttpResponse(HTTP_1_1, HttpResponseStatus.OK, Unpooled.copiedBuffer(json,
                        CharsetUtil.UTF_8));
                resp.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/json; charset=UTF-8");
                resp.headers().setInt(HttpHeaderNames.CONTENT_LENGTH, resp.content().readableBytes());
                resp.headers().set(HttpHeaderNames.CONNECTION, HttpHeaderValues.KEEP_ALIVE);
                return resp;
            } catch (RuntimeException e) {
                return new DefaultFullHttpResponse(HTTP_1_1, HttpResponseStatus.BAD_REQUEST);
            }
        }
    }
}
