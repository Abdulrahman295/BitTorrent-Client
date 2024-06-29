# BitTorrent Client
<div align="center">
   <img src="https://github.com/Abdulrahman295/BitTorrent-Client/assets/89452130/5fde1d70-4f7f-4120-b10d-a362ef294b3a" alt="Bittorrent Logo" width="150" height="150" />
  <p align="center"> 🚀 A BitTorrent client that implements core functionalities of the BitTorrent protocol, including file parsing, tracker communication, and peer-to-peer file downloading.🌐📁</p>
   <p align="center">
      <img src="https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++"/>
      <img src="https://img.shields.io/badge/BitTorrent-4A154B?style=for-the-badge" alt="BitTorrent"/>
      <img src="https://img.shields.io/badge/License-MIT-green.svg?style=for-the-badge" alt="License"/>
   </p>
</div>

## 📑 Table of Contents
- [Features](#-features)
- [Architecture](#️-architecture)
- [Prerequisites](#️-prerequisites)
- [Installation](#️-installation)
- [Usage](#-usage)
- [License](#-license)

## ✨ Features
- **Parse .torrent Files**: Reads and interprets .torrent files, extracting metadata such as tracker URLs, file sizes, piece lengths, and hash values for data integrity.

- **Tracker Communication**: Communicates with trackers to announce the client's presence and obtain a list of peers available for downloading the file.

- **BitTorrent Protocol**: Implements the BitTorrent protocol, allowing the client to connect to peers, perform handshakes, and exchange pieces of the file.

-  **Multi-threaded Downloading**: Supports downloading pieces from multiple peers simultaneously, optimizing the download speed and efficiency and reassembling them to form the complete file.

- **Piece Selection**: Allows downloading of specific pieces of a file.

- **Piece Verification**: Ensures data integrity by verifying downloaded pieces against the hash values provided in the .torrent file.

## ⚙️ Architecture
<div align="center">
   <img src="https://github.com/Abdulrahman295/BitTorrent-Client/assets/89452130/d3a77d50-ec27-4220-b0f3-93cbfb9ae084" alt="Architecture Diagram" />
</div>

## ☑️ Prerequisites
To build and run this project, you need:
- C++23 compatible compiler
- CMake (version 3.13 or higher)

## 🗺️ Installation
1) Clone this repo:
```Bash
git clone https://github.com/Abdulrahman295/BitTorrent-Client.git
```

2) Build the project:
```Bash
cmake . -B build
cmake --build build
```

## 💡 Usage
### Decode Command
Decode bencoded values, supporting four data types: strings, integers, arrays, and dictionaries.
```bash
./bittorrent decode <bencoded values>
```
Example:
```bash
./bittorrent decode '5:hello'
# Output: "hello"

./bittorrent decode 'i3e'
# Output: 3

./bittorrent decode 'l4:spam4:eggse'
# Output: ["spam", "eggs"]

./bittorrent decode 'd3:cow3:moo4:spam4:eggse'
# Output: {"cow": "moo", "spam": "eggs"}
```

### Info Command 
Parse a torrent file and calculate info and piece hashes.
```Bash
./bittorrent info <torrent file>
```
Example:
```Bash
./bittorrent info sample.torrent
# Output:
# Tracker URL: http://bittorrent-test-tracker.codecrafters.io/announce
# Length: 92063
# Info Hash: d69f91e6b2ae4c542468d1073a71d4ea13879a7f
# Piece Length: 32768
# Piece Hashes:
#   e876f67a2a8886e8f36b136726c30fa29703022d
#   6e2275e604a0766656736e81ff10b55204ad8d35
```

### Peers Command 
Discover peers' IP addresses from a torrent file.
```Bash
./bittorrent peers <torrent file>
```
Example:
```Bash
./bittorrent peers sample.torrent
# Output:
# 178.62.82.89:51470
# 165.232.33.77:51467
# 178.62.85.20:51489
```


### Handshake Command
Establish a TCP connection with a peer.
```Bash
./bittorrent handshake <torrent file> <peer_ip>:<peer_port>
```
Example:
```Bash
./bittorrent handshake sample.torrent 178.62.82.89:51470
# Output: Peer ID: 0102030405060708090a0b0c0d0e0f1011121314
```

### Download Piece Command 
Download specific pieces of the torrent file.
```Bash
 download_piece -o <output file> <torrent file> <piece_index>
```
Example:
```Bash
./bittorrent download_piece -o /tmp/test-piece-0 sample.torrent 0
# Output: Piece 0 downloaded to /tmp/test-piece-0.
```

### Download Command  
Download the entire file and save it to disk.
```Bash
 download -o <output file> <torrent file>
```
Example:
```Bash
./bittorrent download -o /tmp/test.txt sample.torrent
# Output: Downloaded sample.torrent to /tmp/test.txt.
```

## 📰 License
This project is licensed under the MIT License. See the `LICENSE` file for more details.
