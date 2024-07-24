# Icemaker-Chiller-K40-CO2-Laser

Simple system for chilling cooling fluid for the K40 CO2 Laser tube.
I found sites on the internet about chilling the K40 laser with a Ice maker. I looked and did a little math and it should be work.
So I bought one at the local Walmart for $98.00 CND and started my test.
I tried with stock with no modes, just pushing the return tube up into the ice forming tray and and the pump into water tank.
IT WORKED. And it was enough of a test to move forward with modding the Ice Maker. 
I used a Cheap temperature controller from China, disabled the circuit board and just let the controller handle turning on and off the compressor
to try and keep the water temperature return between 15-20 degrees C.

By the pictures you can find here under Pictures, you will see what the unit looks like. I ran this way for over a year while I tinkered with 
a Arduino system to controll the Chiller, Water Flow, Door Alarms and laser lockout.
I pretty much had the code written, but wanted a bigger display for this system. So looked hard for a cheap simple module to get my bigger display.
The OctaPentaVeega VGA Shield was the winner. Very simple, three wires, power, gnd and data. 


# Notes
Just for ones knowledge, you bypass all the built in controls on the ice maker. You control the heat pump/compressor with a relay for the temperature control.
The process of pumping the water from the laser tube to the top water bucket in the ice maker where the cooling fingers are, cools the water. As the water fills the top bucket, it falls to the bottom bucket and is pumped out to the laser tube. I have a water flow indicator on the input flow to monitor the water temperature coming in. This is the temperature the controller uses to controls the heat pump. The water pump should always be running, cooling or not.
### Inlet/Outlet are in reference to the laser tube. Not the cooler system.




# Update 
Hand built the circuit to run the K40 Co2 Laser cooler system(Code V6.0) and built the OctaPentaVeega VGA Shield (mono) for the display(Old VGA monitor).
Easy to see across my little shop and the cooler has never let me down. 
![Screenshot 2024-07-24 111251](https://github.com/user-attachments/assets/9a9cad58-1962-4d91-9657-df44336ace87)
![Screenshot 2024-07-24 111325](https://github.com/user-attachments/assets/40305bf1-5a5d-4601-b0bc-5ce330576fc6)
![Screenshot 2024-07-24 111416](https://github.com/user-attachments/assets/40d12ab4-6d2c-4d84-b41e-21f1e7d21716)
![Screenshot 2024-07-24 111444](https://github.com/user-attachments/assets/66fb1828-48ec-4d6b-ab71-6170b6326284)
![Screenshot 2024-07-24 111511](https://github.com/user-attachments/assets/d1be73b3-65b9-4c3f-bfbe-c5ae2d46ff21)
![Screenshot 2024-07-24 111536](https://github.com/user-attachments/assets/8df83108-ada1-424b-96b0-b3c1c0a37f92)



