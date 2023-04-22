import db from "@root/database";

export async function clean(seconds: number): Promise<void> {
    const date = new Date();
    const current = Math.floor(date.getTime() / 1000); // seconds
    const deleteFrom = current - seconds;
    const cleanTimeout = 60; // seconds

    if (current % cleanTimeout !== 0) {
        return;
    }

    await db.query(`DELETE FROM Users WHERE UNIX_TIMESTAMP(createdAt) < ${deleteFrom}`);
    await db.query(`DELETE FROM Notes WHERE UNIX_TIMESTAMP(createdAt) < ${deleteFrom}`);
    await db.query(`DELETE FROM Viewers WHERE UNIX_TIMESTAMP(createdAt) < ${deleteFrom}`);

    console.log(`[*] Cleaned at ${date}`);
}
