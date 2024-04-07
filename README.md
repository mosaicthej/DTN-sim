# Delay Tolerant Network Simulator

delay-tolerant network simulator and visualization (using curses)

## Intro

Design and implement a protocol for data collection in delay-tolerant networks.

The protocol should be able to handle the following scenarios:
-
- Embedded devices (e.g. sensors) does not have enough power 
to transmit very far.
- Apocalyptic scenario where the network is fragmented and nodes can only 
communicate with nodes in their vicinity.
- Situation where an actual device not involved, such as X wants to give a
message to Y (invite to dinner), then X would relay the message through other
shared aquaintances.
- Simulating spread of a virus/information in a social network.

We chose to be based on Epidemic protocols, 
but with some modifications to: **nodes**, **base stations**, **messages** 
to reduce the overhead and reduce duplicate traffic.

This generator would take parameters:
- 
- **N**: Number of devices/stations (all nodes)
- **D**: Number of base stations (destinations)
    Note that, if two destinations are in the same vicinity, 
    they can send messages to each other.
- **K**: Number of timestamps to simulate (steps)
- **B**: Buffer space for each node
- **


## 
