package ru.ctf.db.filter;

public record Eq<T>(String fieldName, T value) implements Filter<T> {
    @Override
    public String sqlPredicate() {
        return fieldName + " = ?";
    }

    @Override
    public String toString() {
        return sqlPredicate();
    }
}
