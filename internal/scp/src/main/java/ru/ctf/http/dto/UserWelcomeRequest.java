package ru.ctf.http.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

public record UserWelcomeRequest(
        @JsonProperty("login") String login,
        @JsonProperty("password") String password
) {
}
