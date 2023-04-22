package ru.ctf.scpql;

public enum PragmaAuth {
    SCPQL,
    TOKEN;

    public static PragmaAuth fromString(String val) {
        if (val.equals("token")) {
            return PragmaAuth.TOKEN;
        }
        return PragmaAuth.SCPQL;
    }
}
