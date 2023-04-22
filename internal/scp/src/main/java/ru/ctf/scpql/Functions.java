package ru.ctf.scpql;

public enum Functions {
    SAVE("save"),
    GET("get");

    private final String value;
    Functions(String val) {
        this.value = val;
    }

    @Override
    public String toString() {
        return this.value;
    }

}
