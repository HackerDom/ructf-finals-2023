package ru.ctf.db.filter;

public class Or extends LogicalJunction {
    private static final String OR = " OR ";

    public Or(Filter<?> left, Filter<?> right) {
        super(OR, left, right);
    }

    public Or(CompositeFilter left, Filter<?> right) {
        super(OR, left, right);
    }

    public Or(CompositeFilter left, CompositeFilter right) {
        super(OR, "0=1", left, right);
    }
}
