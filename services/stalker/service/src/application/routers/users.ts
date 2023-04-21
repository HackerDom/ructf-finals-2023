import { Users } from '@root/services';
import { parseJson } from '@root/utils';
import { withAppContext } from '@root/application/context';

import { ServiceRouter } from './router';

const router = new ServiceRouter();

router.withErrorHandler();
router.withAuthToken();

router.get('/profile/:name', withAppContext(async (ctx, appCtx) => {
    const { name } = ctx.req.param();

    return ctx.jsonT(
        await Users.get(appCtx, { name }),
    );
}));

router.post('/register', withAppContext(async (ctx, appCtx) => {
    const { name, password } = await parseJson(ctx.req.body);

    return ctx.jsonT(
        await Users.register(appCtx, { name, password }),
    );
}));

router.post('/login', withAppContext(async (ctx, appCtx) => {
    const { name, password } = await parseJson(ctx.req.body);

    return ctx.jsonT(
        await Users.login(appCtx, { name, password }),
    );
}));

router.post('/logout', withAppContext(async (ctx, appCtx) => {
    return ctx.jsonT(
        await Users.logout(appCtx, {}),
    );
}));

export default router;
