package ru.ctf.http;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.Arrays;
import java.util.Map;
import java.util.stream.Collectors;

import io.netty.handler.codec.http.DefaultFullHttpResponse;
import io.netty.handler.codec.http.FullHttpRequest;
import io.netty.handler.codec.http.FullHttpResponse;
import io.netty.handler.codec.http.HttpMethod;
import io.netty.handler.codec.http.HttpResponseStatus;
import org.graalvm.collections.Pair;

import static io.netty.handler.codec.http.HttpVersion.HTTP_1_1;

public class Router {
    private final Map<Pair<URI, HttpMethod>, HandlingStrategy> path2strategy;

    private static final Router INSTANCE = new Router(
            // content
            new AuthorizationHandling.ContentStrategy(),
            new RootPageHandling.ContentStrategy(),
            // action
            new AuthorizationHandling.ActionStrategy(),
            new RootPageHandling.ActionStrategy()
    );

    public static Router getInstance() {
        return INSTANCE;
    }

    private Router(HandlingStrategy... strategies) {
        this.path2strategy = Arrays.stream(strategies).collect(
                Collectors.toUnmodifiableMap(
                        st -> Pair.create(st.uri(), st.method()),
                        st -> st
                )
        );
    }

    public FullHttpResponse route(FullHttpRequest request) throws URISyntaxException {
        var strategy = path2strategy.get(Pair.create(new URI(request.uri()), request.method()));
        if (null == strategy) {
            return new DefaultFullHttpResponse(HTTP_1_1, HttpResponseStatus.NOT_IMPLEMENTED);
        }
        try {
            return strategy.handle(request);
        } catch (Exception e) {
            return new DefaultFullHttpResponse(HTTP_1_1, HttpResponseStatus.INTERNAL_SERVER_ERROR);
        }
    }

}
