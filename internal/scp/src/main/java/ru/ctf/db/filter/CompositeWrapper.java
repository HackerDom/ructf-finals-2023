package ru.ctf.db.filter;

import java.util.List;

public class CompositeWrapper<T> implements CompositeFilter {
    private final Filter<T> filter;

    public CompositeWrapper(Filter<T> filter) {
        this.filter = filter;
    }

    @Override
    public List<Filter<?>> filters() {
        return List.of(filter);
    }

    @Override
    public String sqlPredicate() {
        return filter.sqlPredicate();
    }
}
