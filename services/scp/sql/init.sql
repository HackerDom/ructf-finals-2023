CREATE SCHEMA IF NOT EXISTS scp;

CREATE TABLE IF NOT EXISTS scp.user
(
    token      text PRIMARY KEY,
    "login"    varchar(128) NOT NULL,
    "password" varchar(256) NOT NULL,
    created_at timestamptz  NOT NULL
);

CREATE UNIQUE INDEX IF NOT EXISTS idx_unique_user_login ON scp.user ("login");

CREATE TABLE IF NOT EXISTS scp.document
(
    doc_id        serial PRIMARY KEY,
    user_name     varchar(128) NOT NULL REFERENCES scp.user (login),
    title         varchar(256) NOT NULL,
    created_at    timestamptz  NOT NULL,
    template_name varchar(128) NOT NULL
);

CREATE TABLE IF NOT EXISTS scp.doc_fields
(
    doc    bigint REFERENCES scp.document (doc_id) ON DELETE CASCADE,
    name   varchar(256) NOT NULL,
    value  text,
    secret bool         NOT NULL DEFAULT false,
    PRIMARY KEY (doc, name)
)
