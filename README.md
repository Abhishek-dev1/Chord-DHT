# CHORD-DHT Complete Documentation

## Table of Contents
1. [Overview](#overview)
2. [What You Built](#what-you-built)
3. [Technology Stack](#technology-stack)
4. [Architecture](#architecture)
5. [How CHORD Works](#how-chord-works)
6. [Building the Project](#building-the-project)
7. [Usage Guide](#usage-guide)
8. [Practical Examples](#practical-examples)
9. [Troubleshooting](#troubleshooting)
10. [References](#references)

---

## Overview

**CHORD** (Chord: A Scalable Peer-to-peer Lookup Service for Internet Applications) is a distributed hash table (DHT) protocol that enables efficient storage and retrieval of key-value pairs in a peer-to-peer network.

### Key Characteristics
- **Distributed**: Data spreads across multiple nodes in a ring topology
- **Scalable**: Works with any number of nodes joining/leaving
- **Efficient**: Uses O(log N) hops to locate any key in a network of N nodes
- **Self-organizing**: Nodes automatically join a ring and maintain consistency

---

## What You Built

A **peer-to-peer distributed key-value store** using the **CHORD protocol** that allows:
- Multiple nodes to form a self-organizing network ring
- Efficient O(log N) lookup of keys across the network
- Automatic data distribution and balancing
- Graceful node addition and removal

### Architecture Overview

```
┌─────────────────────────────────────────┐
│        DHT Node (CHORD Ring)            │
├─────────────────────────────────────────┤
│  ┌──────────────────────────────────┐  │
│  │   NodeDht Class                  │  │
│  │  ├─ Node ID (SHA-1 hash)         │  │
│  │  ├─ Successor/Predecessor        │  │
│  │  ├─ Finger Table (160 entries)   │  │
│  │  ├─ Data Dictionary              │  │
│  │  └─ Successor List               │  │
│  └──────────────────────────────────┘  │
│            ↕                            │
│  ┌──────────────────────────────────┐  │
│  │   Network Layer                  │  │
│  │  ├─ TCP Sockets                  │  │
│  │  ├─ Port Management              │  │
│  │  └─ Inter-node Communication     │  │
│  └──────────────────────────────────┘  │
│            ↕                            │
│  ┌──────────────────────────────────┐  │
│  │   Utility Functions              │  │
│  │  ├─ SHA-1 Hashing                │  │
│  │  ├─ Finger Table Calculation     │  │
│  │  ├─ Ring Arithmetic              │  │
│  │  └─ Key Routing                  │  │
│  └──────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

---

## Technology Stack

### Why These Technologies?

| Technology | Purpose | Why Used |
|-----------|---------|----------|
| **C++11** | Core language | High performance, low latency, manual control |
| **OpenSSL (-lcrypto)** | SHA-1 hashing | Consistent node IDs, uniform key distribution |
| **POSIX Threads (-lpthread)** | Multithreading | Concurrent network I/O + maintenance tasks |
| **TCP Sockets** | Network transport | Reliable, ordered packet delivery |

### Technical Rationale

- **SHA-1 Hashing**: Maps 2^160 potential identifiers → uniform ring distribution
- **Ring topology**: O(log N) lookup beats O(N) broadcast
- **Finger table**: Exponential spacing enables binary search on ring
- **Threads**: Keep network listening separate from user commands
- **C++**: Performance critical for distributed systems

---

## Architecture

### File Structure

```
NodeDht (Core Data Structure)
├── nodeClass/nodeDht.{cpp,h}     ← Node state + DHT logic
├── utility/
│   ├── headerFile/headers.h       ← All system includes
│   ├── Max_lim.h                  ← Ring size (2^160)
│   ├── Function/              
│   │   └── functions.{cpp,h}      ← put, get, create, join, leave
│   ├── initialization/            
│   │   └── init.{cpp,h}           ← CLI interface, command parser
│   ├── nodeDhtUtill/              
│   │   └── utill.{cpp,h}          ← SHA-1, ring math helpers
│   └── portFunction/              
│       └── port.{cpp,h}           ← Socket management
├── main.cpp                        ← Entry point (calls initialize)
└── Makefile                        ← Build configuration
```

### Core Components

#### 1. **NodeDht (nodeClass/)**
The heart of your implementation - represents a single node in the CHORD ring.

**Key Data Structures:**
- `id` (long long int): SHA-1 hash of the node's IP:Port
- `predecessor` & `successor`: Links to adjacent nodes in the ring
- `fingerTable`: Array of pointers to distant nodes (exponentially spaced)
- `successorList`: List of next few successors for fault tolerance
- `dictionary`: Map storing key-value pairs this node is responsible for

**Key Methods:**
- `findSuccessor(key)`: Locates which node should store a key
- `closestPrecedingNode(key)`: Finds best intermediate node for routing
- `fixFingers()`: Maintains finger table entries
- `stabilize()`: Repairs ring connections after node joins/leaves
- `notify()`: Alerts predecessor/successor about topology changes

#### 2. **Utility Functions (utility/)**

**Max_lim.h**: Defines ring size (2^160 for SHA-1 hashes)

**Port Functions (portFunction/port.cpp)**
- Creates TCP server socket for inter-node communication
- Auto-finds available ports
- Manages socket lifecycle

**Initialization (initialization/init.cpp)**
- Sets up the DHT node
- Provides CLI interface for node operations
- Command parser for user interactions

**Helper Functions (nodeDhtUtill/utill.cpp)**
- SHA-1 hashing for node IDs and keys
- Finger table calculations
- Key responsibility calculation
- Ring arithmetic operations

**Operations (Function/functions.cpp)**
- `create()`: Creates new CHORD ring (first node)
- `join(ip, port)`: Node joins existing ring
- `put(key, value)`: Stores key-value pair
- `get(key)`: Retrieves value for a key
- `leave()`: Gracefully removes node from ring
- `stabilize()`: Repairs ring structure
- `fixFingers()`: Updates finger table entries
- `notify()`: Updates predecessor/successor relationships

---

## How CHORD Works

### 1. Node Identification
Each node gets a unique ID based on SHA-1 hash of its IP address and port number:
```
NodeID = SHA-1(IP:Port) % 2^160
```

### 2. Ring Topology
Nodes are arranged in a logical ring from 0 to 2^160 - 1:
```
         [Node A]
        /        \
   [Node D]    [Node B]
        \        /
         [Node C]
```
Each node's successor is the next node clockwise on the ring.

### 3. Key Storage
A key is stored on the successor node of its hash:
```
Key "mykey" → SHA-1("mykey") → Find successor of this hash → Store there
```

### 4. Finger Table (Fast Lookup)
Each node maintains a finger table with log(N) entries pointing to:
```
Finger[1] → successor of (NodeID + 2^0)
Finger[2] → successor of (NodeID + 2^1)
Finger[3] → successor of (NodeID + 2^2)
...
Finger[160] → successor of (NodeID + 2^159)
```
This enables logarithmic lookup instead of linear.

### 5. Maintenance Operations
- **Stabilize()**: Runs periodically to repair ring connections
- **FixFingers()**: Updates finger table entries
- **CheckPredecessor()**: Verifies predecessor is alive

### Key Algorithms

#### Consistent Hashing
```
Position on ring = SHA-1(IP:Port) mod 2^160
For key k: responsible node = successor(SHA-1(k) mod 2^160)
```
✅ **Benefit**: Minimal data movement on node join/leave

#### Ring Traversal (Finger Table)
```
Finger[i] points to successor(NodeID + 2^(i-1))
Query for key? Jump to closest finger, repeat 1-160 times max
```
✅ **Benefit**: O(log N) hops instead of O(N)

#### Stabilization Protocol
```
Every Δt: ask successor for its predecessor
If new predecessor found between me and current successor:
    Update pointer, sync data
```
✅ **Benefit**: Automatically repairs ring after failures/joins

#### Predecessor Notification
```
When node joins: Send notify to successors
Successors update their predecessor pointers
Keys get redistributed to new node if needed
```
✅ **Benefit**: Decentralized consistency maintenance

### Performance Characteristics

| Metric | Value | Implication |
|--------|-------|-------------|
| Lookup hops | O(log N) | 1000 nodes = ~10 hops |
| State per node | O(log N) | Only needs 160 finger entries |
| Join time | O(log² N) | Fast integration of new nodes |
| Scalability | Unbounded | Works from 2 to millions of nodes |

---

## Building the Project

### Prerequisites
Ensure you have the required development libraries:
```bash
sudo apt-get install -y libssl-dev build-essential
```

### Compile the Source Code
Navigate to the project directory and build:
```bash

make clean
make
```

**Expected Output:**
```
g++ -std=c++11 -I./headerFile ... -c main.cpp
g++ -std=c++11 -I./headerFile ... -c nodeClass/nodeDht.cpp
... (more compilation messages)
g++ main.o init.o functions.o port.o nodeDht.o utill.o -o chorddht -lcrypto -lpthread
```

**Result:** Executable `chorddht` created (~260KB)

### Clean Build Artifacts
To remove compiled object files:
```bash
make clean
```

This removes all `.o` files but keeps the `chorddht` executable.

### Full Clean
```bash
rm -f chorddht *.o
```

---

## Usage Guide

### Running Single Node

#### Step 1: Start the Program
```bash
./chorddht
```

**Output:**
```
listening at port number 52446
> 
```

The program auto-assigns a random port.

#### Step 2: Create a Ring (Initialize DHT)
```
> create
```

This creates the first node/ring. Your node becomes its own successor and predecessor.

#### Step 3: Check Node Information
```
> port
52446

> printstate
```

Shows your:
- Node ID (SHA-1 hash of IP:Port)
- Successor node pointer
- Predecessor node pointer
- Finger table (160 entries)
- Successor list

#### Step 4: Store Data (Put Operation)
```
> put mykey myvalue
> put username john
> put email john@example.com
```

Stores key-value pairs on this node's storage.

#### Step 5: Retrieve Data (Get Operation)
```
> get mykey
```

Retrieves the value associated with the key.

#### Step 6: List All Keys
```
> print
```

Shows all keys and values stored on this node.

#### Step 7: Exit Gracefully
```
> leave
```

Cleanly removes the node from the ring and exits.

### Running Multiple Nodes

#### Setup: Open 3 Terminals

**Terminal 1: Node A (First/Main Node)**
```bash
./chorddht
> create
> port
```

Note the port number (e.g., **5000**).

```
> put data1 "Terminal 1 Data"
> printstate
```

**Terminal 2: Node B (Second Node)**
```bash
./chorddht
> port
```

Note different port (e.g., **5001**).

```
> join localhost 5000
```

Connects to Node A's ring.

```
> put data2 "Terminal 2 Data"
> printstate
```

**Terminal 3: Node C (Third Node)**
```bash
./chorddht
> join localhost 5000
```

Joins Node A's ring and connects to the existing nodes.

```
> put data3 "Terminal 3 Data"
```

### Command Reference

#### Commands for Single Arguments

| Command | Usage | Example | Result |
|---------|-------|---------|--------|
| `create` | `create` | `> create` | Creates first DHT ring |
| `print` | `print` | `> print` | Lists all local keys |
| `printstate` | `printstate` | `> printstate` | Shows node info, fingers, ring state |
| `port` | `port` | `> port` | Displays current listening port |
| `help` | `help` | `> help` | Shows all available commands |
| `leave` | `leave` | `> leave` | Exits ring gracefully |

#### Commands with Arguments

| Command | Usage | Example | Result |
|---------|-------|---------|--------|
| `port` | `port <number>` | `> port 6000` | Changes port (only before joining ring) |
| `get` | `get <key>` | `> get mykey` | Retrieves value for key |
| `join` | `join <ip> <port>` | `> join localhost 5000` | Joins ring at specified node |
| `put` | `put <key> <value>` | `> put name john` | Stores key-value pair |

---

## Practical Examples

### Data Distribution

When you store data across multiple nodes:

```
Node A (Port 5000):
  - NodeID: 12345...
  - put mydata "hello"      → May store on Node B or C (depends on hash)
  - put config "settings"   → May store locally or on another node

Node B (Port 5001):
  - Stores keys whose hash successor is Node B
  - get mydata              → Queries ring, finds Node C has it, retrieves value

Node C (Port 5002):
  - Stores keys whose hash successor is Node C
```

### Ring Topology Visualization

```
After 3 nodes join:

          ┌─────────────┐
          │   Node A    │
          │ Port: 5000  │
          │ ID: 12345   │
          └─────────────┘
                 ↑    ↓
           successor prediction
                 ↓    ↑
          ┌─────────────┐
          │   Node C    │ ←── Node A's successor
          │ Port: 5002  │
          │ ID: 45678   │
          └─────────────┘
                 ↑    ↓
          ┌─────────────┐
          │   Node B    │
          │ Port: 5001  │
          │ ID: 34567   │
          └─────────────┘

Each node maintains:
- Successor pointer (next node clockwise)
- Predecessor pointer (previous node clockwise)
- 160-entry Finger Table (for efficient routing)
- Successor List (backup successors for fault tolerance)
```

### Workflow Examples

#### Scenario 1: Local Store & Retrieve
```
Terminal 1:
> create
> put author "Abhishek"
> put version "1.0"
> print
  Keys: author, version
> get author
  Value: Abhishek
```

#### Scenario 2: Distributed Queries
```
Terminal 1:
> create
> put server_ip "192.168.1.1"
> port
  5000

Terminal 2:
> join localhost 5000
> get server_ip
  Value: 192.168.1.1  (retrieved from Terminal 1)

Terminal 1:
> print
  Keys: server_ip (may appear here after stabilization)
```

#### Scenario 3: Three-Node Ring with Data
```
Terminal 1: create → put x 10 → put y 20
Terminal 2: join localhost 5000 → put z 30
Terminal 3: join localhost 5000 → put w 40

Terminal 2: get x → retrieves from appropriate node
Terminal 3: get y → retrieves from appropriate node
Terminal 1: get z, get w → retrieves from other nodes
```

#### Scenario 4: Data Flow Example
```
[Terminal 2] put "mykey" "myvalue"
    ↓
hash("mykey") → NodeID: 87123...
    ↓
Find successor of 87123... in ring
    ↓
If successor is Terminal 1's node → send "mykey":"myvalue" there
    ↓
[Terminal 1] now stores the key-value pair
    ↓
[Terminal 2] get "mykey"
    ↓
Queries Terminal 1's node → returns "myvalue"
```

---

## Troubleshooting

### Issue: "Already on the ring"
```
> create
Already on the ring
```
**Solution:** Node is already in a ring. Use `leave` first, then create/join.

### Issue: "Error! Not in the ring!"
```
> put mykey value
Error! Not in the ring!
```
**Solution:** Must run `create` or `join` before storing data.

### Issue: Can't change port after joining
```
> port 6000
:( port number can't be changed!
```
**Solution:** Can only change port when not in ring. Use `leave` first.

### Issue: Connection refused when joining
```
> join 192.168.1.100 5000
(no response/timeout)
```
**Solution:**
- Verify the target node is running: Check it's listening on port 5000
- Use correct IP: Try `localhost` if on same machine
- Check firewall: Ensure port 5000 is accessible

### Issue: "Invalid Command"
```
> put key
Invalid Format
```
**Solution:** Commands need correct number of arguments. Use `help` to verify syntax.

### Issue: Key not found
```
> get nonexistent_key
(empty response)
```
**Solution:** Key doesn't exist or is on another node (which has crashed). This is expected.

---

## Advanced Tips

### Multiple Rings on Same Machine
You can run multiple DHT rings on the same machine using different ports:

```bash
# Terminal 1:
./chorddht
> create

# Terminal 2:
./chorddht
> port 6000
> create

# Now you have 2 independent rings
```

### Querying Remote Nodes
```bash
# Terminal 1 (Machine A):
./chorddht
> create
> put data1 "from machine A"
> port
  5000

# Terminal 2 (Machine B):
./chorddht
> join <Machine-A-IP> 5000
> get data1  # Retrieves from Machine A
  Value: from machine A
```

### Monitor Ring Stability
```bash
> printstate
> (wait 5 seconds)
> printstate
```

Ring state should stabilize as nodes run stabilize() and fixFingers() maintenance tasks.

### Performance Notes

- **Lookup Time**: O(log N) where N = number of nodes
- **Join Time**: O(log² N)
- **Storage Per Node**: O(log N) for finger table
- **Network Traffic**: Few messages per operation in stabilized ring

**Example**: 1,024 nodes = ~10 hops to find any key

---

## References

Your implementation is based on:
1. [MIT CHORD Paper](https://pdos.csail.mit.edu/papers/ton:chord/paper-ton.pdf) - Original research
2. [CMU DHT Lecture](https://www.cs.cmu.edu/~dga/15-744/S07/lectures/16-dht.pdf) - Educational overview

### Key Features of Your Implementation

1. **Fully Distributed**: No central authority needed
2. **Self-Healing**: Ring repairs itself automatically
3. **Scalable**: Works with thousands of nodes
4. **Fault Tolerant**: Successor lists handle node crashes
5. **Educational**: Great reference for DHT concepts
6. **Real Networking**: Actual TCP sockets, not simulation
