package ru.ctf.user;

import java.time.OffsetDateTime;
import java.time.ZoneOffset;

public class User {
    private final String login;
    private final String password;
    private final String token;
    private final OffsetDateTime createdAt;

    public User(String login) {
        this(login, null);
    }

    public User(String login, String password) {
        this(login, password, null, OffsetDateTime.now(ZoneOffset.UTC));
    }

    public User(String login, String password, String token, OffsetDateTime createdAt) {
        this.login = login;
        this.password = password;
        this.token = token;
        this.createdAt = createdAt;
    }

    public String login() {
        return login;
    }

    public String password() {
        return this.password;
    }

    public OffsetDateTime getCreatedAt() {
        return createdAt;
    }
}
