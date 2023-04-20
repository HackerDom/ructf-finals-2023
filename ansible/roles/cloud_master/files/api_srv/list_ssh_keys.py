import do_api

from do_tokens import DO_TOKENS

for token_name, token in DO_TOKENS.items():
    print(token_name)

    keys = sorted(do_api.get_ssh_keys(token).items())

    for key_id, key_name in keys:
        print(key_id, key_name)
