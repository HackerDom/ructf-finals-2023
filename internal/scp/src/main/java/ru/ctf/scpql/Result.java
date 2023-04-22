package ru.ctf.scpql;

import com.fasterxml.jackson.annotation.JsonProperty;

public record Result(
        @JsonProperty("type") ResultType Type,
        @JsonProperty("body") String body) {
    @Override
    public String toString() {
        return "Result{" +
                "Type=" + Type +
                ", body='" + body + '\'' +
                '}';
    }
}
