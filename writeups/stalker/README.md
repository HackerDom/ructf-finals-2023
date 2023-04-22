# RuCTF Finals 2023 | stalker

## Description

The service was named after [Stalker](https://en.wikipedia.org/wiki/Stalker_(1979_film)) — a 1979 Soviet science fiction movie directed by [Andrei Tarkovsky](https://en.wikipedia.org/wiki/Andrei_Tarkovsky).

## Overview

Let's look at the Notes schema:

```sql
MariaDB [mariadb]> DESCRIBE Notes;
+-----------+------------+------+-----+---------+----------------+
| Field     | Type       | Null | Key | Default | Extra          |
+-----------+------------+------+-----+---------+----------------+
| title     | text       | NO   | UNI | NULL    |                |
| content   | text       | NO   |     | NULL    |                |
| visible   | tinyint(1) | NO   | PRI | 1       |                |
| ownerId   | int(11)    | NO   | MUL | NULL    |                |
| id        | int(11)    | NO   | PRI | NULL    | auto_increment |
| createdAt | datetime   | NO   |     | NULL    |                |
| updatedAt | datetime   | NO   |     | NULL    |                |
+-----------+------------+------+-----+---------+----------------+
7 rows in set (0.003 sec)
```

We see that PRIMARY KEY is `(visible, id)`, where `id` is AUTO_INCREMENT column. But the table constructor sets MyISAM as the default engine for all new tables. Let's read more about MyISAM engine on [MySQL reference manual](https://dev.mysql.com/doc/refman/8.0/en/myisam-storage-engine.html):

> Internal handling of one AUTO_INCREMENT column per table is supported. MyISAM automatically updates this column for INSERT and UPDATE operations. This makes AUTO_INCREMENT columns faster (at least 10%). Values at the top of the sequence are not reused after being deleted. (When an AUTO_INCREMENT column is defined as the last column of a multiple-column index, reuse of values deleted from the top of a sequence does occur.) 

If AUTO_INCREMENT column is not the first in PRIMARY KEY, it may be non-unique for different INSERTs. Instead, it calculates from existing ids as follows:

```sql
SET newId AS MAX(SELECT id FROM table)
```

It leads to the vulnerability.

## Vulnerability

Let's look at the request path of the application. There are several middlewares in front of the route handlers:

```
-> withErrorHandler
    -> withAdditionalHeaders
        -> withAuthToken
            -> <route handler>
                -> <service method>
```

`withAuthToken()` middleware is used for authentication. On each request it extracts JWT token from headers and tries to verify it. If the verification was successful, the middleware loads `User` instance from the database and stores it in the request context.

But what happens next? The control flow jumps to the actual route handler. For example:

```typescript
router.post('/:title/share', withAppContext(async (ctx, appCtx) => {
    const { title } = ctx.req.param();
    const { viewer } = await parseJson(ctx.req.body);

    return ctx.jsonT(
        await Notes.share(appCtx, { title, viewer }),
    );
}));
```

Please notice a tiny `await` here. The request waits for a JSON body and decodes it. Reading in JavaScript is non-blocking, so the waiting Promise will be returned back to to the event pool. When the body is completely read, the Promise returns back and the handler calls a service method.

Remember, that `withAuthToken()` middleware has put the _User_ instance into the context, and it still presents in the application memory. And the service method uses `ctx.user` in order to get owned notes for this user:

```typescript
async share(ctx: AppContext, req: ShareNoteRequest): Promise<ShareNoteResponse> {
    if (!isShareNoteRequest(req)) {
        throw new ValidationError('invalid request message');
    }

    if (ctx.user === null) {
        throw new LoginRequiredError('you are not logged in');
    }

    const note = ctx.user.notes.find(
        note => note.title === req.title,
    );

    if (typeof note === 'undefined') {
        throw new OwnerMismatchError('you should own this note');
    }

    if (note.visible) {
        throw new ValidationError('note is visible');
    }

    const viewer = await User.findOne({ where: { name: req.viewer } });

    if (viewer === null) {
        throw new UserNotFoundError('viewer not found');
    }

    if (viewer.name !== ctx.user.name) {
        await note.addViewer(viewer);
    }

    return {};
}
```

What if the attacker would delete the actual _Note_ from the database **before** the entering the service method?

```
withAuthToken() -> <delete note> -> Notes.share()
```

That's is: all notes, have been loaded from the database before, will be presented in the context. And the service will assume that these note is owned by _User_, even if they were deleted from the database.

Suppose that checker had put a _Note_ into the database before the `Notes.share()` phase. Due to the AUTO_INCREMENT behaviour (described above), the checker's note should have the same id as the attacker's note.

And the attacker will be able to share the checker's note to an arbitrary user:

```typescript
const note = ctx.user.notes.find(
    note => note.title === req.title,
);

const viewer = await User.findOne({ where: { name: req.viewer } });

await note.addViewer(viewer); // here `note` has id of the checker's note
```

## Example attack

1. Create two users: `attacker` and `viewer`

2. [`attacker`]: Create a note

3. [`attacker`]: Begin new request for sharing the created note to viewer, send http headers only

4. [`attacker`]: Delete the created note from the database

5. Wait until the checker puts a note with flag

6. [`attacker`]: Contunue the request and send body data

7. The checker's note will be shared to the viewer

8. [`viewer`]: View the note

Example exploit: [sploit.py](/sploits/stalker/sploit.py)

P.S. After successful stealing of the checker's note, we could easily do the opposite operation: delete `viewer` from note's viewers. This improvement will remain Viewers table unchanged.

## Patch

Just delete `visible` from PRIMARY KEY and AUTO_INCREMENT fields will become unique.
