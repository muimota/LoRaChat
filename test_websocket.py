# WS client example

import asyncio
import websockets
from time import sleep 

async def hello():
    uri = "ws://192.168.4.1/msg"
    async with websockets.connect(uri) as websocket:
        
        for i in range(1000):
        	msg = 'test|test {}'.format(i)
        	await websocket.send(msg)
        	print(f"> {msg}")
        	
        	response = await websocket.recv()
        	print(f"< {response}")
        	sleep(2)
	
asyncio.get_event_loop().run_until_complete(hello())
