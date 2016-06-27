#!/bin/sh
# Run it from as user where qemu will be running

set -e

sudo apt-get install qemu uml-utilities -y
sudo tunctl -t tap0 -u $(whoami)
