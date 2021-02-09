import aiohttp
import asyncio
import datetime


async def fetch(session, url):
    async with session.get(url) as response:
        return await response.text()


async def send_req():
    async with aiohttp.ClientSession() as session:
        payload = {'camp1': 'valoare1', 'camp2': 'valoare2'}
        headers = {'Content-Type': 'application/json',
                   'Connection': 'close', 'Accept-Encoding': ''}
        # async with session.post('http://localhost:54300/resurse/1', headers=headers, data=payload) as resp:
        async with session.post('http://localhost:45600/resurse/1', headers=headers, data=payload) as resp:
        # async with session.post('http://httpbin.org/post', headers=headers, data=payload) as resp:
            # data = await resp.json()
            # print(resp)
            # print()
            # print(data)
            return 1 if resp.status == 200 else 0


async def main():
    start = datetime.datetime.now()
    print(sum(await asyncio.gather(*(send_req() for _ in range(10)))))
    end = datetime.datetime.now()
    print(int((end - start).total_seconds() * 1000000))
if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())


# import urllib3
# import json
# import datetime

# http = urllib3.PoolManager()
# headers = {"Content-Type": "application/json", "Connection": "close", "Accept-Encoding": "", "Accept":"*"}
# payload = json.dumps({'camp1': 'valoare1', 'camp2': 'valoare2'})
# start = datetime.datetime.now()
# r = http.request('POST', 'http://httpbin.org/post', headers=headers, body=payload)
# end = datetime.datetime.now()
# print(int((end - start).total_seconds() * 1000000))
# print(r.status)
# print(r.data.decode('ascii'))
