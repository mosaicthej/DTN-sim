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

## Endogenous Variables (Parameters)

This generator would take parameters:
-
- **N**: Number of devices/stations (all nodes)
- **D**: Number of base stations (destinations)
    Note that, if two destinations are in the same vicinity, 
    they can send messages to each other.
- **K**: Number of timestamps to simulate (steps)
- **V**: Speed of the nodes (the length of the speed vector in 2D),
    Note that the speed is node-specific, so this V is a upper bound.
    Each node would have a speed vector with a random length between 0 and V.
    at each timestamp.
- **B**: Buffer space for each node
- **R**: Communication range, once two nodes has distance less than R, 
    they can communicate.

## Observing Objectives

Among many exogenous variables generated by this model, 
    and potential observable emergent properties, 
    we have 2 measures that is the main concern as it can be 
    metrics to evaluate the *goodness* of the model:
-
### Delivery Ratio

The ratio of messages that reach the destination.

This reflects the reliability of the protocol. 

The more messages are delivered,
    the more reliable the protocol is in this delay tolerant network.

Usually, to increase the relability, redundancy is needed, and traffic amount 
    would increase. We'd like to have as many members to 
    have a message as possible. 

By having more members in the network to hold the message, we effectively
    reduces the chance of a node who got near the destination but does not have
    the message to deliver.

However, as $P(\text{Node A has message}) = \frac{1}{N}$, it is likely that
    $P(\text{Message fails to reach destination})$ follows a binomial or poisson
    distribution, which, after an optimal point (by taking the *critical value*
    of the rate of change in delivery ratio), or equivalently, the *point of
    inflection*, would marks the beginning of *diminishing returns*. 
    Further additions of redundancy of the message within the network 
    would yield less and less increase in delivery ratio.

I suspect that the model's behavior would be sensitive to each input parameter,
    and the delivery ratio would be a function of all the parameters.
    It might be possible to find a close-form solution to such *sweet spot*,
    using a system of ODEs.

### Overhead Ratio


