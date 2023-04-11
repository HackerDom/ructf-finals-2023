import { Notes } from '@root/services';

import { ServiceRouter } from './router';

const router = new ServiceRouter();

router.withErrorHandler();
router.withAuthCookie();

router.get('/:title', async (ctx) => {
    const { title } = ctx.req.param();

    return ctx.jsonT(
        await Notes.get(ctx.env, { title }),
    );
});

router.post('/', async ctx => {
    const { title, visible, content } = await ctx.req.json();

    return ctx.jsonT(
        await Notes.create(ctx.env, { title, visible, content }),
    );
});

router.post('/:title/share', async ctx => {
    const { title } = ctx.req.param();
    const { viewer } = await ctx.req.json();

    return ctx.jsonT(
        await Notes.share(ctx.env, { title, viewer }),
    );
});

router.post('/:title/deny', async ctx => {
    const { title } = ctx.req.param();
    const { viewer } = await ctx.req.json();

    return ctx.jsonT(
        await Notes.deny(ctx.env, { title, viewer }),
    );
});

router.post('/:title/destroy', async ctx => {
    const { title } = ctx.req.param();

    return ctx.jsonT(
        await Notes.destroy(ctx.env, { title }),
    );
});

export default router;
