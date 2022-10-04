import os
pid = os.getpid()
import cantor
print("Init with pid = ",pid)

import asyncio
import wss
import httpsrv

cantor.Config(AutoExit=True)


async def Main():
    httpsrv.StartServer()
    await  wss.StartServer()
    while cantor.IsHostRunning():
        await asyncio.sleep(0.5)

asyncio.run(Main())
