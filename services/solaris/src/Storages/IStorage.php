<?php

interface IStorage
{
    function load_password(string $username): string|null;
    function store_password(string $username, string $password): void;

    function load_key(string $username): Key|null;
    function store_key(string $username, Key $key): void;

    function load_ciphertext(string $id): Matrix|null;
    function store_ciphertext(string $id, Matrix $ciphertext): void;
}
