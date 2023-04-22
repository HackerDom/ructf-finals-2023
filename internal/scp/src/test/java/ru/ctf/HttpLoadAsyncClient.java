package ru.ctf;

import java.net.URI;
import java.util.ArrayList;
import java.util.Collection;

import io.netty.bootstrap.Bootstrap;
import io.netty.buffer.Unpooled;
import io.netty.channel.Channel;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioSocketChannel;
import io.netty.handler.codec.http.DefaultFullHttpRequest;
import io.netty.handler.codec.http.HttpHeaderNames;
import io.netty.handler.codec.http.HttpHeaderValues;
import io.netty.handler.codec.http.HttpMethod;
import io.netty.handler.codec.http.HttpRequest;
import io.netty.handler.codec.http.HttpVersion;

public class HttpLoadAsyncClient {
    static final String URL = System.getProperty("url", "http://127.0.0.1:8088/");
    private static final int CONNECTION_COUNT = 1000;
    private static final int THREAD_COUNT = 12;

    public static void main(String[] args) throws Exception {
        var startedAt = System.currentTimeMillis();
        URI uri = new URI(URL);
        String scheme = uri.getScheme() == null ? "http" : uri.getScheme();
        String host = uri.getHost() == null ? "127.0.0.1" : uri.getHost();
        int port = uri.getPort();
        if (port == -1) {
            if ("http".equalsIgnoreCase(scheme)) {
                port = 80;
            } else if ("https".equalsIgnoreCase(scheme)) {
                port = 443;
            }
        }

        if (!"http".equalsIgnoreCase(scheme) && !"https".equalsIgnoreCase(scheme)) {
            System.err.println("Only HTTP(S) is supported.");
            return;
        }

        /*
        // Configure SSL context if necessary.
        final boolean ssl = "https".equalsIgnoreCase(scheme);
        final SslContext sslCtx;
        if (ssl) {
            sslCtx = SslContextBuilder.forClient()
                    .trustManager(InsecureTrustManagerFactory.INSTANCE).build();
        } else {
            sslCtx = null;
        }
        */

        // Prepare the HTTP request.
        HttpRequest request = new DefaultFullHttpRequest(
                HttpVersion.HTTP_1_1, HttpMethod.GET, uri.getRawPath(), Unpooled.EMPTY_BUFFER);
        request.headers().set(HttpHeaderNames.HOST, host);
        request.headers().set(HttpHeaderNames.CONNECTION, HttpHeaderValues.CLOSE);
        request.headers().set(HttpHeaderNames.ACCEPT_ENCODING, HttpHeaderValues.GZIP);

        // Configure the client.
        EventLoopGroup group = new NioEventLoopGroup(THREAD_COUNT);
        try {
            Bootstrap b = new Bootstrap();
            b.group(group)
                    .channel(NioSocketChannel.class)
                    .handler(new HttpLoadAsyncClientInitializer(null));


            var futures = requestsFutures(CONNECTION_COUNT, b, host, port, request);
            for (Channel ch : futures) {
                // Wait for the server to close the connection.
                ch.closeFuture().sync();
            }
            System.out.println(futures.size() + " requests was executed");
            double time = (System.currentTimeMillis() - startedAt) / 1000D;
            System.out.println(time + "sec");
        } finally {
            // Shut down executor threads to exit.
            group.shutdownGracefully();
        }
    }

    private static Collection<Channel> requestsFutures(
            int connCount,
            Bootstrap b,
            String host,
            int port,
            HttpRequest request
    ) throws InterruptedException {
        var res = new ArrayList<Channel>((int) (connCount * 1.25));
        for (int i = 0; i < connCount; i++) {
            // Make the connection attempt.
            Channel ch = b.connect(host, port).sync().channel();

            // Send the HTTP requests.
            ch.writeAndFlush(request);
            res.add(ch);
        }
        return res;
    }
}
