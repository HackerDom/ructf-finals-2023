package ru.ctf.scpql;

public class EmptyValue extends Value<String>{
    public EmptyValue() {
        this.type = Type.EMPTY;
    }
}
