package ru.ctf.doc;

import java.time.OffsetDateTime;

import ru.ctf.TemplatesHolder;
import ru.ctf.user.User;

public class Document {
    private final String title;
    private final User owner;
    private final Content content;
    private final OffsetDateTime createdAt;
    private final String templateName;
    public Document(String title, User owner, Content content, OffsetDateTime createdAt) {
        this.title = title;
        this.owner = owner;
        this.content = content;
        this.createdAt = createdAt;
        this.templateName = TemplatesHolder.getRandomName();
    }

    public Document(String title, User owner, Content content, OffsetDateTime createdAt, String templateName) {
        this.title = title;
        this.owner = owner;
        this.content = content;
        this.createdAt = createdAt;
        this.templateName = templateName;
    }


    public String title() {
        return title;
    }

    public User owner() {
        return owner;
    }

    public Content content() {
        return content;
    }

    public OffsetDateTime createdAt() {
        return createdAt;
    }

    public String templateName() {
        return templateName;
    }
}
