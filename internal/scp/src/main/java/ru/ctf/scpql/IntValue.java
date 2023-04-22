package ru.ctf.scpql;

import java.util.Map;

import ru.ctf.doc.Document;

public class IntValue extends Value<Integer>{
    public IntValue(String val) {
        this.value = Integer.parseInt(val);
        this.type = Type.INT;
    }

    @Override
    public String toString() {
        return Integer.toString(this.value);
    }

    @Override
    public Result toResult(Map<String, Value<?>> vars, Map<String, Document> docs) {
        return new Result(ResultType.PLAIN, this.toString());
    }
}
