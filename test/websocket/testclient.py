import asyncio
import websockets

async def hello():
    async with websockets.connect('ws://localhost:8765') as websocket:
        for i in range(10000):
            name = "name="+str(i) #input("What's your name? ")
            await websocket.send(name)
            print("> {}".format(name))

            greeting = await websocket.recv()
            print("< {}".format(greeting))

asyncio.get_event_loop().run_until_complete(hello())

