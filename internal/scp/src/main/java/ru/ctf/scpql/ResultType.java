package ru.ctf.scpql;

public enum ResultType {
    PLAIN("plain"),
    HTML("html");

    private final String value;
    ResultType(String val) {
        this.value = val;
    }

    @Override
    public String toString() {
        return this.value;
    }
}

