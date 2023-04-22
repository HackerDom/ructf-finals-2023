package ru.ctf.db;

import java.sql.SQLException;

public class CleanerTool {

    public static final int ALLEVIATION_INTERVAL = 120; // minutes

    public static void clean() {
        var deleteDocumentsByOwner = """
                DELETE FROM scp.document as doc
                USING scp."user" as u
                WHERE EXTRACT(minute FROM now() - u.created_at) > ?;
                """;
        var deleteOldUsers = """
                DELETE FROM scp."user" as u
                WHERE EXTRACT(minute FROM now() - u.created_at) > ?;
                """;
        try (
                var conn = PostgresqlDbcpDataSource.getConnection();
                var st = conn.prepareStatement(deleteDocumentsByOwner + deleteOldUsers);
        ) {
            st.setInt(1, ALLEVIATION_INTERVAL);
            st.setInt(2, ALLEVIATION_INTERVAL);
            st.executeUpdate();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
}
