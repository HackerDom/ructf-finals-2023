package ru.ctf.doc.dao;

import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.Timestamp;
import java.time.ZoneOffset;
import java.util.ArrayList;
import java.util.List;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import ru.ctf.db.PostgresqlDbcpDataSource;
import ru.ctf.db.filter.CompositeFilter;
import ru.ctf.doc.Content;
import ru.ctf.doc.DocWithOwner;
import ru.ctf.doc.Document;
import ru.ctf.scpql.StrValue;
import ru.ctf.user.User;

public class DocumentDAO implements DAO<Long, Document> {
    private static final DAO<Long, Document> INSTANCE = new DocumentDAO();

    public static DAO<Long, Document> getInstance() {
        return INSTANCE;
    }

    @Override
    public Long save(Document document) {
        var sql = """
                INSERT INTO scp.document (user_name, title, created_at, template_name)
                VALUES (?,?,?,?)
                RETURNING doc_id;
                """;

        try (
                var conn = PostgresqlDbcpDataSource.getConnection();
        ) {
            var st1 = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS);
            st1.setString(1, document.owner().login());
            st1.setString(2, document.title());
            st1.setTimestamp(3, Timestamp.from(document.createdAt().toInstant()));
            st1.setString(4, document.templateName());
            st1.executeUpdate();
            var contentSql = new StringBuilder("INSERT INTO scp.doc_fields (doc, name, value, secret) VALUES");
            try (var rs = st1.getGeneratedKeys()) {
                for (int i = 0; i < document.content().getFields().size() - 1; i++) {
                    contentSql.append("\n").append("(?,?,?,?),");
                }
                contentSql.append("\n").append("(?,?,?,?);");
                st1 = conn.prepareStatement(contentSql.toString());
                if (rs.next()) {
                    var docId = rs.getLong(1);
                    var shift = 0;
                    for (var entry : document.content().getFields().entrySet()) {
                        st1.setLong(shift + 1, docId);
                        st1.setString(shift + 2, entry.getKey());
                        st1.setString(shift + 3, entry.getValue().getValue().toString());
                        st1.setBoolean(shift + 4, entry.getValue().isSecret());
                        shift += 4;
                    }
                    st1.executeUpdate();
                    st1.close();
                    return docId;
                }

                throw new RuntimeException();
            }
        } catch (SQLException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public List<Long> delete(CompositeFilter filter) {
        return List.of();
    }

    @Override
    public List<Document> list(Integer offset, Integer limit, CompositeFilter filter) {
        var selectDoc = """
                SELECT doc.title, doc.user_name, df.value, doc.created_at
                FROM scp.document doc
                INNER JOIN scp.doc_fields df on doc.doc_id = df.doc
                """;
        var where = "WHERE " + filter.sqlPredicate();
        var limitAndOffsetSql = " OFFSET " + offset + " LIMIT " + limit;

        var filters = filter.filters();
        try (
                var conn = PostgresqlDbcpDataSource.getConnection();
                var st = conn.prepareStatement(selectDoc + where + limitAndOffsetSql + ";")
        ) {
            for (int i = 1; i <= filters.size(); i++) {
                st.setObject(i, filters.get(i - 1).value());
            }
            var documents = new ArrayList<Document>();
            try (var rs = st.executeQuery()) {
                while (rs.next()) {
                    documents.add(
                            new Document(
                                    rs.getString(1),
                                    new User(rs.getString(2)),
                                    new Content(),
                                    rs.getTimestamp(4).toLocalDateTime().atOffset(ZoneOffset.UTC)
                            )
                    );
                }
                return documents;
            } catch (SQLException e) {
                throw new RuntimeException(e); //FIXME
            }
        } catch (SQLException e) {
            throw new RuntimeException(e); // FIXME
        }
    }

    @Override
    public List<DocWithOwner> getAll(Integer offset, Integer limit) {
        var sql = """
                SELECT doc.doc_id, doc.user_name
                FROM scp.document doc
                OFFSET ?
                LIMIT ?;""";
        try (
                var conn = PostgresqlDbcpDataSource.getConnection();
                var st = conn.prepareStatement(sql)
        ) {
            st.setInt(1, offset);
            st.setInt(2, limit);
            var docs = new ArrayList<DocWithOwner>();
            try (var rs = st.executeQuery()) {
                while (rs.next()) {
                    docs.add(
                            new DocWithOwner(
                                    rs.getLong(1),
                                    rs.getString(2)
                            )
                    );
                }
                return docs;
            } catch (SQLException e) {
                throw new RuntimeException(e); //FIXME
            }
        } catch (SQLException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public @Nullable Document get(Long id) {
        var sql = """
                SELECT doc.title, doc.user_name, doc.created_at, doc.template_name, df.name, df.value, df.secret
                FROM scp.document doc
                INNER JOIN scp.doc_fields df on doc.doc_id = df.doc
                WHERE doc.doc_id=?;""";
        try (
                var conn = PostgresqlDbcpDataSource.getConnection();
                var st = conn.prepareStatement(sql)
        ) {
            st.setLong(1, id);
            return executeSingleDocumentSelect(st);

        } catch (SQLException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public @Nullable Document find(CompositeFilter filter) {
        var select = """
                SELECT doc.title, doc.user_name, df.value, doc.created_at
                FROM scp.document doc
                INNER JOIN scp.doc_fields df on doc.doc_id = df.doc
                """;
        var where = "WHERE " + filter.sqlPredicate();
        var limit = " LIMIT 1;";

        var filters = filter.filters();
        try (
                var conn = PostgresqlDbcpDataSource.getConnection();
                var st = conn.prepareStatement(select + where + limit)
        ) {
            for (int i = 1; i <= filters.size(); i++) {
                st.setObject(i, filters.get(i - 1).value());
            }
            return executeSingleDocumentSelect(st);
        } catch (SQLException e) {
            throw new RuntimeException(e);
        }
    }

    @NotNull
    private static Document executeSingleDocumentSelect(PreparedStatement st) throws SQLException {
        try (var rs = st.executeQuery()) {
            if (rs.next()) {
                var content = new Content();
                var doc = new Document(
                        rs.getString(1),
                        new User(rs.getString(2)),
                        content,
                        rs.getTimestamp(3).toLocalDateTime().atOffset(ZoneOffset.UTC),
                        rs.getString(4)
                );
                do {
                    var val = new StrValue(rs.getString(6));
                    val.setSecret(rs.getBoolean(7));
                    content.setField(rs.getString(5), val);
                } while (rs.next());
                return doc;
            } else {
                throw new RuntimeException("asdad"); //FIXME
            }
        }
    }


    private DocumentDAO() {
    }
}
