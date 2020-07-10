# Configurations

## General

The file `demo.json` is an example of DAQling general configuration.

The file `demo-metrics-manager.json` extends `demo.json`, showcasing how to handle Python scripts automatically with `daqpy`.

## (NEW) Configurations Dictionary

The introduction of the `nodetree` library, comes with some new required configurations along with the previously described general configuration:

- Tree definition
- Rules of the Finite State Machine (FSM)

These three configurations are kept in separate JSON files; therefore a dictionary is required in order to point to their relative paths (different sets of configuration files can be stored in separated folders).

`demo-dict.json` is an example of such a dictionary, that can be used with `daqtree`.

Note that the example file uses the same `demo.json` that can be used with `daqpy`.

### Tree definition

In order to define the tree, with is automatically imported, one needs to define for each node a `name`, an optional `type` (see Rules of FSM), and, if the node is a controller, a `children` array.

Entries inside the `children` array, are just nodes, as previously described.

See `control-tree.json` for an example.

### Rules of FSM

The rules are divided in two parts: `fsm` and `order`.

In the `fsm` part, an array of allowed "actions" is assigned for every possible "state".

In the `order` part, it's possible to specify the order in which a certain action is executed on the different optional `type`s, as specified in the tree definition. The order is top-first.