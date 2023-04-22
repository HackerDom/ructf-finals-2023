package ru.ctf.doc.dao;

import java.util.List;

import javax.naming.OperationNotSupportedException;

import org.jetbrains.annotations.Nullable;
import ru.ctf.db.filter.CompositeFilter;

public interface DAO<Key, Entity> {
    Key save(Entity entity);

    List<Key> delete(CompositeFilter filter);

    default Entity update(Entity entity) throws OperationNotSupportedException {
        throw new OperationNotSupportedException("Update is not supported");
    }

    List<Entity> list(Integer offset, Integer limit, CompositeFilter filter);
     <T> List<T> getAll(Integer offset, Integer limit);

    @Nullable
    Entity get(Key key);

    @Nullable
    Entity find(CompositeFilter filter);
}
