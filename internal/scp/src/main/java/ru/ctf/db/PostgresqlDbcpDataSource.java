package ru.ctf.db;

import java.sql.Connection;
import java.sql.SQLException;
import java.util.Properties;

import javax.sql.DataSource;

import org.apache.commons.dbcp2.BasicDataSourceFactory;

public class PostgresqlDbcpDataSource {

    private static DataSource dataSource = null;


    static {
        try {
            var props = new Properties();
            props.load(PostgresqlDbcpDataSource.class.getClassLoader().getResourceAsStream("database.properties"));
            dataSource = BasicDataSourceFactory.createDataSource(props);
        } catch (Exception ex) {
            ex.printStackTrace();
            System.exit(1);
        }
    }

    public static DataSource getDataSource() {
        return dataSource;
    }

    public static Connection getConnection() throws SQLException {
        return dataSource.getConnection();
    }

    private PostgresqlDbcpDataSource() {
    }
}
