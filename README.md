# Mood - MUD engine based on libevent, exposing gopher/telnet interface

This is a possibly stupid idea, which is running in my head since a good
while now.

I recently got to know the [Gopher protocol][gopher], and I realized it
still works quite well on command-line browsers like [lynx][]. I really
loved it, as I'm very fond of textual interfaces, simplicity and minimalism.

Gopher supports links of type "telnet", which are basically a pair Host +
TCP port, which will be used for a telnet session by the well-behaved
client.

The idea is using a Gopher hierarchy as it was a map, and Telnet sessions
for communication. For example, let's define a Dungeon with a filesystem
hierarchy:

    /
    ├── map
    │   ├── corridor
    │   │   ├── east -> ../loot-room/
    │   │   └── south -> ../entrance/
    │   ├── entrance
    │   │   ├── dwarf -> ../../npc/dwarf
    │   │   └── north -> ../corridor/
    │   └── loot-room
    │       ├── ogre -> ../../npc/ogre
    │       └── west -> ../corridor/
    └── npc
        ├── dwarf
        └── ogre

Rooms can have links to other rooms to define geography. Each NPC is a
process which allows interaction via Telnet protocol.

Details are still foggy… but this should give you the idea…

[gopher]: <https://en.wikipedia.org/wiki/Gopher_protocol>
[lynx]: <http://lynx.browser.org/>
