package ru.ctf.http;

import java.net.URISyntaxException;

import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.SimpleChannelInboundHandler;
import io.netty.handler.codec.http.FullHttpRequest;

public class RequestHandler extends SimpleChannelInboundHandler<FullHttpRequest> {

    private static final Router router = Router.getInstance();

    @Override
    protected void channelRead0(ChannelHandlerContext ctx, FullHttpRequest msg) throws URISyntaxException {
        var response = router.route(msg);
        ctx.write(response);
    }

    @Override
    public void channelReadComplete(ChannelHandlerContext ctx) {
        ctx.flush();
        //The close is important here in an HTTP request as it sets the Content-Length of a
        //response body back to the client.
        ctx.close();
    }
}
