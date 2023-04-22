package ru.ctf.db.filter;

public interface Predicate {
    /**
     * @return sql-syntax predicate with '?' placeholder and without trailing '\n' for using in WHERE clause
     */
    String sqlPredicate();
}
