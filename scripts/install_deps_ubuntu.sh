#!/bin/bash
sudo dpkg --add-architecture i386
sudo apt update
sudo apt-get install gcc-multilib
sudo apt install zlib1g-dev:i386 libc6-dev:i386