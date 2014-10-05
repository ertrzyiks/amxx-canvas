amxx-canvas [![Build Status](https://travis-ci.org/ertrzyiks/amxx-canvas.svg?branch=master)](https://travis-ci.org/ertrzyiks/amxx-canvas)
===========

## About
Proof-of-concept code which aim to provide programable displayer inside Half-Life multiplayer gameplay. 


## Prerequisites
- Half Life Dedicated Server or listen server created by game client (New Game)
- AMX mod X - http://www.amxmodx.org/

## Engine limits
Each pixel of displayer must be permanent or temporary entity. It looks like we cant render more than ~250 visible object in the same time. Maximal number of entities in general is configurable, but i cant find way to increase this visiblity limit. 

That reduced planned resolution to 28x8 for text experiments.


## Installation
Extract repository package into server root path.

## Demo
See @[youtube](https://www.youtube.com/watch?v=UcpbDoaZ3Qc&list=PLz0GOppD1_jVxQFYLYhKwIexxiDB238rI)
