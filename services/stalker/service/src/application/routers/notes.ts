import { Notes } from '@root/services';
import { parseJson } from '@root/utils';
import { withAppContext } from '@root/application/context';

import { ServiceRouter } from './router';

const router = new ServiceRouter();

router.withErrorHandler();
router.withAdditionalHeaders();
router.withAuthToken();

router.get('/:title', withAppContext(async (ctx, appCtx) => {
    const { title } = ctx.req.param();

    return ctx.jsonT(
        await Notes.get(appCtx, { title }),
    );
}));

router.post('/', withAppContext(async (ctx, appCtx) => {
    const { title, visible, content } = await parseJson(ctx.req.body);

    return ctx.jsonT(
        await Notes.create(appCtx, { title, visible, content }),
    );
}));

router.post('/:title/share', withAppContext(async (ctx, appCtx) => {
    const { title } = ctx.req.param();
    const { viewer } = await parseJson(ctx.req.body);

    return ctx.jsonT(
        await Notes.share(appCtx, { title, viewer }),
    );
}));

router.post('/:title/deny', withAppContext(async (ctx, appCtx) => {
    const { title } = ctx.req.param();
    const { viewer } = await parseJson(ctx.req.body);

    return ctx.jsonT(
        await Notes.deny(appCtx, { title, viewer }),
    );
}));

router.post('/:title/destroy', withAppContext(async (ctx, appCtx) => {
    const { title } = ctx.req.param();

    return ctx.jsonT(
        await Notes.destroy(appCtx, { title }),
    );
}));

export default router;
