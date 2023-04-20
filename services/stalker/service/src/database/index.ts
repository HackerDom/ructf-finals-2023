import { Sequelize } from 'sequelize';

export default new Sequelize({
    dialect: 'mariadb',

    ssl: false,
    host: Bun.env.DATABASE_HOST ?? 'localhost',
    port: 3306,

    database: Bun.env.DATABASE_NAME ?? 'mariadb',
    username: Bun.env.DATABASE_USERNAME ?? 'mariadb',
    password: Bun.env.DATABASE_PASSWORD ?? 'mariadb',

    pool: {
        min: 8,
        max: 32,
        idle: 10_000, // 10 seconds
        acquire: 30_000, // 30 seconds
    },

    retry: {
        max: 5,
        timeout: 10_000, // 10 seconds
    },

    omitNull: true,
    typeValidation: true,
    logging: false,

    define: { engine: 'MyISAM' },
});
