package ru.ctf.scpql;

public enum Specials {
    USER_SPACE("@userspace"),
    TITLE("@title");

    private final String value;
    Specials(String val) {
        this.value = val;
    }

    @Override
    public String toString() {
        return this.value;
    }
}
