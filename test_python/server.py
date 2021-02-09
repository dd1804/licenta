from aiohttp import web

async def post_handler(request):
    return web.Response(text='abcd')

app = web.Application()
app.add_routes([web.post('/resurse/1', post_handler)])

web.run_app(app, port=54300)