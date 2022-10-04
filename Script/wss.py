import cantor
import asyncio
import datetime
import random
import websockets

async def SesionHandler(websocket, path):
    print("Session started")
    while True:
        now = datetime.datetime.utcnow().isoformat() + 'Z'
        await websocket.send(now)
        await asyncio.sleep(random.random() * 0.4)

#start_server = websockets.serve(time, '127.0.0.1', 5678)

async def StartServer():
    await websockets.serve(SesionHandler, '127.0.0.1', 5678)

