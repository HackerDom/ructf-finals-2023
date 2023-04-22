package ru.ctf.user.dao;

import java.sql.SQLException;
import java.sql.Statement;
import java.sql.Timestamp;
import java.time.ZoneOffset;
import java.util.Collections;
import java.util.List;
import java.util.UUID;

import org.jetbrains.annotations.Nullable;
import ru.ctf.db.PostgresqlDbcpDataSource;
import ru.ctf.db.filter.CompositeFilter;
import ru.ctf.doc.dao.DAO;
import ru.ctf.user.DupUserException;
import ru.ctf.user.User;

public class UserDAO implements DAO<String, User> {

    private static final DAO<String, User> INSTANCE = new UserDAO();

    public static DAO<String, User> getInstance() {
        return INSTANCE;
    }

    @Override
    public String save(User user) {
        var sql = """
                INSERT INTO scp.user (token, login, password, created_at)
                VALUES (?, ?, ?, ?);
                """;
        try (
                var conn = PostgresqlDbcpDataSource.getConnection();
        ) {
            var st = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS);
            var uuid = UUID.randomUUID().toString();
            st.setString(1, uuid);
            st.setString(2, user.login());
            st.setString(3, user.password());
            st.setTimestamp(4, Timestamp.from(user.getCreatedAt().toInstant()));
            st.executeUpdate();
            return uuid;
        } catch (SQLException e) {
            if (e.toString().contains("duplicate key")) {
                throw new DupUserException();
            }
            throw new RuntimeException(e);
        }
    }

    @Override
    public List<String> delete(CompositeFilter filter) {
        return Collections.emptyList();
    }

    @Override
    public List<User> list(Integer offset, Integer limit, CompositeFilter filter) {
        return Collections.emptyList();
    }

    @Override
    public <T> List<T> getAll(Integer offset, Integer limit) {
        return Collections.emptyList();
    }

    @Override
    public @Nullable User get(String s) {
        var sql = """
                SELECT "user".token, "user".login, "user".password, "user".created_at FROM scp.user
                WHERE "user".token = ?;
                """;
        try (
                var conn = PostgresqlDbcpDataSource.getConnection();
        ) {
            var st = conn.prepareStatement(sql);
            st.setString(1, s);
            try (var rs = st.executeQuery()) {
                if (rs.next()) {
                    return new User(
                            rs.getString(2),
                            rs.getString(3),
                            rs.getString(1),
                            rs.getTimestamp(4).toLocalDateTime().atOffset(ZoneOffset.UTC)
                    );
                }
            }
        } catch (SQLException e) {
            throw new RuntimeException(e);
        }
        return null;
    }

    @Override
    public @Nullable User find(CompositeFilter filter) {
        return null;
    }

    private UserDAO() {

    }
}
