package ru.ctf.db.filter;

import java.util.List;

public interface CompositeFilter extends Predicate {
    List<Filter<?>> filters();
}
