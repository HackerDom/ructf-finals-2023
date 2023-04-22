package ru.ctf.db.filter;

import java.util.ArrayList;
import java.util.List;

public abstract class LogicalJunction implements CompositeFilter {
    protected final String junction;
    protected final List<Filter<?>> filters;
    protected final StringBuilder sqlBuilder = new StringBuilder("(");

    protected LogicalJunction(String junction, Filter<?> left, Filter<?> right) {
        this.junction = junction;

        this.filters = List.of(left, right);
        this.sqlBuilder.append(left.sqlPredicate())
                .append(this.junction)
                .append(right.sqlPredicate());
    }

    protected LogicalJunction(String junction, CompositeFilter left, Filter<?> right) {
        this.junction = junction;

        this.filters = new ArrayList<>(left.filters());
        this.filters.add(right);

        appendJunction(left);
        this.sqlBuilder.append(right.sqlPredicate());
    }

    protected LogicalJunction(String junction, String crutchCondition, CompositeFilter left, CompositeFilter right) {
        this.junction = junction;

        this.filters = new ArrayList<>(left.filters());
        this.filters.addAll(right.filters());

        appendJunction(left);
        appendJunction(right);
        sqlBuilder.append(crutchCondition);
    }

    @Override
    public String sqlPredicate() {
        return sqlBuilder.append(")").toString();
    }

    @Override
    public String toString() {
        return sqlPredicate();
    }

    @Override
    public List<Filter<?>> filters() {
        return filters;
    }

    private void appendJunction(CompositeFilter compositeFilter) {
        for (Filter<?> f : compositeFilter.filters()) {
            this.sqlBuilder.append(f.sqlPredicate()).append(junction);
        }
    }
}
