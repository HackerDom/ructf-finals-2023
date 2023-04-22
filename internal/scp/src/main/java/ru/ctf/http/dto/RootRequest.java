package ru.ctf.http.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

public record RootRequest(
        @JsonProperty("token") String token,
        @JsonProperty("query") String query
) {
}
