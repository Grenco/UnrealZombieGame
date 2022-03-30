# Zombie Game
This game tasks you to survive for as long as possible against hordes of enemies, following the classic "Zombies" playstyle.

You are thrown straight into the action as you have to find powerups and shelter to survive against the ever-increasing hordes.

There are 2 types of enemy: smart AI-players who can find the player, shoot from under cover and find cover to heal when damaged; and rolling tracker bots which will explode when they get too close.

This game was developed in Unreal Engine 5 while following "Unreal Engine 4 Mastery: Create Multiplayer Games with C++" by Tom Looman on Udemy, with my own custom twists added. The course can be found [here](https://www.udemy.com/course/unrealengine-cpp/). It's a great way to get started with Unreal Development.

## Project setup
The project should be ready to go out of the box, just download Unreal Engine 5, clone the repository and build the game to play.

## Multiplayer
The code also includes replication for full multiplayer support (except for some of the weapons). Only 2-player mode currently works.

To play with someone else: 
1. Make sure you both have a copy of the game 
2. Both start the game and press the \` key on the keyboard (the one at the top left below the escape key) to bring up the console. 
3. For the host, type `open P_TestMap?listen`
4. On the client's machine, open the console and type `open 127.0.0.1:7777` to join the game

*Note: This only works for 2 computers connected to the same network.* 
