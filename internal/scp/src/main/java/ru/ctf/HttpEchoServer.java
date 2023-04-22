package ru.ctf;


import java.io.IOException;
import java.sql.SQLException;
import java.util.Properties;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelOption;
import io.netty.channel.ChannelPipeline;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.handler.codec.http.HttpObjectAggregator;
import io.netty.handler.codec.http.HttpServerCodec;
import ru.ctf.db.CleanerTool;
import ru.ctf.db.PostgresqlDbcpDataSource;
import ru.ctf.http.RequestHandler;

public class HttpEchoServer {

    private final int port;
    private final int bosses;
    private final int workers;
    private final int maxContentLength;

    public HttpEchoServer(int port, int bosses, int workers, int maxContentLength) {
        this.port = port;
        this.bosses = bosses;
        this.workers = workers;
        this.maxContentLength = maxContentLength;
    }


    /**
     * @see <a href=https://netty.io/wiki/user-guide-for-4.x.html#wiki-h3-6>Netty start guide</a>
     */
    public void run() throws Exception {
        EventLoopGroup bossGroup = new NioEventLoopGroup(bosses);
        EventLoopGroup workerGroup = new NioEventLoopGroup(workers);
        try {
            ServerBootstrap b = new ServerBootstrap();
            b.group(bossGroup, workerGroup)
                    .channel(NioServerSocketChannel.class)
                    .childHandler(new ChannelInitializer<SocketChannel>() {
                        @Override
                        public void initChannel(SocketChannel ch) {
                            ChannelPipeline pipeline = ch.pipeline();
                            pipeline.addLast(new HttpServerCodec());
                            pipeline.addLast(new HttpObjectAggregator(maxContentLength));
                            pipeline.addLast(new RequestHandler());
                        }
                    })
                    .option(ChannelOption.SO_BACKLOG, 1024)
                    .childOption(ChannelOption.SO_KEEPALIVE, true);

            ChannelFuture f = b.bind(port).sync();

            System.out.println("Server successfully started");
            f.channel().closeFuture().sync();
        } finally {
            workerGroup.shutdownGracefully();
            bossGroup.shutdownGracefully();
        }
    }

    public static HttpEchoServer getInstance() {
        try {
            var prop = new Properties();
            prop.load(HttpEchoServer.class.getClassLoader().getResourceAsStream("server.properties"));
            int port = Integer.parseInt(prop.getProperty("sever.port", "8081"));
            int bosses = Integer.parseInt(prop.getProperty("server.bosses", "2"));
            int workers = Integer.parseInt(prop.getProperty("server.workers", "2"));
            int maxContentLength = Integer.parseInt(prop.getProperty("server.maxContentLength", "1024"));
            return new HttpEchoServer(port, bosses, workers, maxContentLength);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public static void main(String[] args) {
        System.out.println("Starting server...");
        try {
            initTemplateHolder();
            checkDbLiveness();
            startCleanDbWorker();
            var server = HttpEchoServer.getInstance();
            server.run();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static void startCleanDbWorker() {
        Executors.newSingleThreadScheduledExecutor()
                .scheduleAtFixedRate(
                        CleanerTool::clean,
                        15,
                        20,
                        TimeUnit.MINUTES
                );
    }

    private static void checkDbLiveness() throws SQLException {
        try (
                var conn = PostgresqlDbcpDataSource.getConnection();
                var st = conn.prepareStatement("SELECT COUNT(*) FROM information_schema.tables;")
        ) {
            assert st.execute();
        }
    }

    private static void initTemplateHolder() {
        TemplatesHolder.getRandomName();
    }
}