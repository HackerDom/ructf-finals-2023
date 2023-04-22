# RuCTF Finals 2023 | SCP

## Description

[SCP Foundation](https://ru.wikipedia.org/wiki/SCP_Foundation)

SCP - service that works with scp documents and provides query language to manipulate documents. This language can create, aggregate documents and hide some information in documents.

## Vulnerability

### First way

1. Let's start analyze from frontend. We can see that frontend sends some queries `@doc <- {(${offset}, ${limit}) -> list};{(@doc) -> result};`. Server returns response that contains some id and owner.
But frontend makes other query `@docId <- ${id};@doc <- {(@docId) -> get};{(@doc) -> result}`, This query returns some text with `ДАННЫЕ УДАЛЕНЫ` and we can realize that here is something hidden.
1. So we have some query language that makes some queries and hide some fields. Let's try to analyze binary file. 

### First way
1. By searching `token` in binary we can find SQL queries and recognize that user identify by token
2. Let's try to find variables linked with user by pattern `@user`. We can find `@userspace`
3. Now we can try to put `@userspace = "{some_username}"` and get document with `{user_name}` owner. Magic! it returns hidden fields.

### Second way
1. By searching .java in binary we can find all Java classes. In a few minutes or hours we can find `Specials.java` and one line below `USER_SPACE({*})`. 
We know that string with prefix `@` is a variable, and we have `Specials.java` with `USER_SPACE`. Let's suppose that `USER_SPACE` is special variable and try to find `@user` and we get `@userspace`. 
1. Now we can try to put `@userspace = "{some_username}"` and get document with `{user_name}` owner. Magic! it returns hidden fields.

## Sploit

1. First of all we need to get documents with owner. It possible to do with query
```
    @docs <- {(0, 50) -> list};
    {(@docs) -> result};
```
2. We have users with their documents. Now we need rewrite `@userspace` and get every document
```
    for doc, user in doc_with_user:
        userspace = f'@userspace <- "{u}";'
        doc = '@doc <- {(' + d + ') -> get};'
        res = '{(@doc) -> result};'
        query = userspace + doc + res
```
3. Make request with rewritten `@userspace`. Congrats!


## Defense

1. We need to forbid change `@userspace` from request. We know that `@userspace` determine by token
2. By searching `TOKEN` we can find enum that contains `SCPQL` and `TOKEN`
3. By searching `scpql` we can find `server.pragmaAuth=scpql`. Little thinking and we recognize that it is auth property
4. We can replace `server.pragmaAuth=scpql` with `server.pragmaAuth=token` and forbid redefinition of `@userspace`