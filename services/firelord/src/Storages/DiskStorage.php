<?php

class DiskStorage implements IStorage
{
    private string $base;

    private const USERS_STORAGE = "users";
    private const KEYS_STORAGE = "keys";
    private const CIPHERTEXTS_STORAGE = "ciphertexts";

    public function __construct(string $directory)
    {
        $this->base = $directory;

        $this->init();
    }

    private function init(): void
    {
        $this->create_storage();

        $this->create_storage(DiskStorage::USERS_STORAGE);
        $this->create_storage(DiskStorage::KEYS_STORAGE);
        $this->create_storage(DiskStorage::CIPHERTEXTS_STORAGE);
    }

    public function load_password(string $username): string|null
    {
        $password = file_get_contents($this->get_password_path($username));

        if ($password === false)
        {
            return null;
        }

        return $password;
    }

    public function store_password(string $username, string $password): void
    {
        $result = file_put_contents($this->get_password_path($username), $password);

        if ($result === false)
        {
            throw new Exception("failed to write password");
        }
    }

    public function load_key(string $username): Key|null
    {
        $key_deflated = file_get_contents($this->get_key_path($username));

        if ($key_deflated === false)
        {
            return null;
        }

        $key_serialized = gzinflate($key_deflated);

        if ($key_serialized === false)
        {
            throw new Exception("failed to inflate key");
        }

        return unserialize($key_serialized);
    }

    public function store_key(string $username, Key $key): void
    {
        $key_serialized = serialize($key);

        $key_deflated = gzdeflate($key_serialized);

        if ($key_deflated === false)
        {
            throw new Exception("failed to deflate key");
        }

        $result = file_put_contents($this->get_key_path($username), $key_deflated);

        if ($result === false)
        {
            throw new Exception("failed to write key");
        }
    }

    public function load_ciphertext(string $id): Matrix|null
    {
        $ciphertext_deflated = file_get_contents($this->get_ciphertext_path($id));

        if ($ciphertext_deflated === false)
        {
            return null;
        }

        $ciphertext_serialized = gzinflate($ciphertext_deflated);

        if ($ciphertext_serialized === false)
        {
            throw new Exception("failed to inflate ciphertext");
        }

        $ciphertext = Matrix::from_json($ciphertext_serialized);

        if ($ciphertext === null)
        {
            throw new Exception("failed to parse ciphertext");
        }

        return $ciphertext;
    }

    public function store_ciphertext(string $id, Matrix $ciphertext): void
    {
        $ciphertext_serialized = $ciphertext->to_json();

        $ciphertext_deflated = gzdeflate($ciphertext_serialized);

        if ($ciphertext_deflated === false)
        {
            throw new Exception("failed to deflate ciphertext");
        }

        $result = file_put_contents($this->get_ciphertext_path($id), $ciphertext_deflated);

        if ($result === false)
        {
            throw new Exception("failed to write ciphertext");
        }
    }

    private function create_storage(string $type = ""): void
    {
        $dirname = sprintf("%s/%s", $this->base, $type);

        if (!is_dir($dirname))
        {
            mkdir($dirname);
        }
    }

    private function get_password_path(string $username): string
    {
        return sprintf("%s/%s/%s.txt", $this->base, DiskStorage::USERS_STORAGE, $username);
    }

    private function get_key_path(string $username): string
    {
        return sprintf("%s/%s/%s.txt", $this->base, DiskStorage::KEYS_STORAGE, $username);
    }

    private function get_ciphertext_path(string $id): string
    {
        return sprintf("%s/%s/%s.txt", $this->base, DiskStorage::CIPHERTEXTS_STORAGE, $id);
    }
}
