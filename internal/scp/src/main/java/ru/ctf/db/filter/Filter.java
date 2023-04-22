package ru.ctf.db.filter;

public interface Filter<T> extends Predicate {

    T value();
}
