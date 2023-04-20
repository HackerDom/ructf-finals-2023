import { Users } from '@root/services';
import { parseJson } from '@root/utils';

import { ServiceRouter } from './router';

const router = new ServiceRouter();

router.withErrorHandler();
router.withAuthToken();

router.all('/profile/:name', async (ctx) => {
    const { name } = ctx.req.param();

    return ctx.jsonT(
        await Users.get(ctx.env, { name }),
    );
});

router.post('/register', async (ctx) => {
    const { name, password } = await parseJson(ctx.req.body);

    return ctx.jsonT(
        await Users.register(ctx.env, { name, password }),
    );
});

router.post('/login', async (ctx) => {
    const { name, password } = await parseJson(ctx.req.body);

    return ctx.jsonT(
        await Users.login(ctx.env, { name, password }),
    );
});

router.post('/logout', async (ctx) => {
    return ctx.jsonT(
        await Users.logout(ctx.env, {}),
    );
});

export default router;
