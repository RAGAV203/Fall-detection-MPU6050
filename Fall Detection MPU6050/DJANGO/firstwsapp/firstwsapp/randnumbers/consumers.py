import random
from channels.generic.websocket import AsyncWebsocketConsumer
import asyncio
import numpy as np
import json
def port_sel():
    #7 Connect to the serial port
    #ser = serial.Serial('COM7', 115200)  # Replace 'COM1' with your serial port and baud rate
    import serial.tools.list_ports

    ports = serial.tools.list_ports.comports()
    serialInst = serial.Serial()

    portsList = []

    for onePort in ports:
        portsList.append(str(onePort))
        print(str(onePort))

    val = 7#input("Select Port: COM")

    for x in range(0,len(portsList)):
       if portsList[x].startswith("COM" + str(val)):
            portVar = "COM" + str(val)
            print(portVar)

            serialInst.baudrate = 112500
            serialInst.port = portVar
            serialInst.open()
            return serialInst
       


class RandomNumberConsumer(AsyncWebsocketConsumer):
    async def connect(self):
        await self.accept()
        print('WebSocket connection established')
        await self.send_random_numbers()  # Start sending random numbers
    
    async def disconnect(self, close_code):
        pass



    async def send_random_numbers(self):
        #while True:
            # context = {
            #         'AcX': np.random.randint(0,100),
            #         'AcY': np.random.randint(0,100),
            #         'AcZ': np.random.randint(0,100),
            #         'GyX': np.random.randint(0,100),
            #         'GyY': np.random.randint(0,100),
            #         'GyZ': np.random.randint(0,100)
            #      } 
        serialInst=port_sel()
        while True:
            if serialInst.in_waiting:
                packet = serialInst.readline()
                print(packet.decode('utf').rstrip('\n'))
                if(packet.decode('utf')[0]=='{'):
                    values = packet.decode('utf').rstrip('\r\n')[1:len(packet.decode('utf').rstrip('\r\n'))-1]
                    values = values.split(',')

        #if not ser.isOpen():
        #    ser.open()
        #print('com3 is open', ser.isOpen())
        # Read the sensor values from the serial port
    # values = ser.readline().decode().strip().split(',')
    # print(values)
        # Extract the accelerometer and gyroscope values
                    AcX = int(values[0])
                    AcY = int(values[1])
                    AcZ = int(values[2])
                    GyX = int(values[3])
                    GyY = int(values[4])
                    GyZ = int(values[5])
                    
    

        # Prepare the context data to pass to the template
                    context = {
                        'AcX': AcX,
                        'AcY': AcY,
                        'AcZ': AcZ,
                        'GyX': GyX,
                        'GyY': GyY,
                        'GyZ': GyZ
                    } 
                    
         
                    print('Sending random number:', context)
                    
                    await self.send(text_data=json.dumps(context))

                    await asyncio.sleep(0.01)  # Adjust the delay as per your requirements
