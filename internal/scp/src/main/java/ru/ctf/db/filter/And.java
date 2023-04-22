package ru.ctf.db.filter;

public class And extends LogicalJunction {
    private static final String AND = " AND ";

    public And(Filter<?> left, Filter<?> right) {
        super(AND, left, right);
    }

    public And(CompositeFilter left, Filter<?> right) {
        super(AND, left, right);
    }

    public And(CompositeFilter left, CompositeFilter right) {
        super(AND, "1=1", left, right);
    }
}
