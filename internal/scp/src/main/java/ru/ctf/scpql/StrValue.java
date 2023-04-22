package ru.ctf.scpql;

public class StrValue extends Value<String>{
    public StrValue(String val) {
        this.value = val;
        this.type = Type.STRING;
    }
}
