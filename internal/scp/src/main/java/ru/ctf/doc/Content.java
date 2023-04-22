package ru.ctf.doc;

import java.util.HashMap;
import java.util.Map;

import ru.ctf.scpql.Value;

public class Content {

    private final Map<String, Value> fields;

    public Content() {
        this.fields = new HashMap<>();
    }

    public void setField(String field, Value val) {
        if (fields.size() >= 7) {
            return;
        }
        this.fields.put(field, val);
    }
    public Map<String, Value> getFields() {
        return this.fields;
    }
}
