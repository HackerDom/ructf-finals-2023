package ru.ctf.scpql;

import java.util.Map;

import ru.ctf.doc.Document;

public abstract class Value<T> {
    protected Type type;
    private boolean isSecret;
    protected T value;
    public void setSecret(boolean sec) {
        this.isSecret = sec;
    }

    public boolean isSecret() {
        return this.isSecret;
    }

    public <T0 extends T> T0 getValue() {
        return (T0) this.value;
    }

    public Type getType() {
        return this.type;
    }

    public String toString() {
        return this.value.toString();
    }

    public Result toResult(Map<String, Value<?>> vars, Map<String, Document> docs) {
        return new Result(ResultType.PLAIN, this.value.toString());
    }
}
