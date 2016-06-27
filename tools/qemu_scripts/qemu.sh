#!/bin/sh

set -e

MACHINE=lm3s6965evb
BLOB_PATH=bin/dcon-virt-mod.bin

HOST_IFACE=tap0
HOST_IP=192.168.200.1

VM_HWADDR=52:54:00:12:34:56
VM_FIXED_IP=192.168.200.200

DHCP_RANGE=192.168.200.100,192.168.200.150,72h
DHCP_LEASE_FILE=/tmp/leases.dhcp

cleanup()
{
    ! sudo ip addr del $HOST_IP/24 dev $HOST_IFACE 2> /dev/null
    ! sudo ip link set $1 down 2> /dev/null
    ! sudo killall dnsmasq 2> /dev/null
    exit 0
}

trap cleanup INT EXIT

sudo ip addr add $HOST_IP/24 dev $HOST_IFACE
sudo ip link set $HOST_IFACE up
sudo dnsmasq  --interface=$HOST_IFACE --bind-interfaces --dhcp-range=$DHCP_RANGE --listen-address=$HOST_IP \
         --dhcp-host=$VM_HWADDR,$VM_FIXED_IP --dhcp-leasefile=DHCP_LEASE_FILE
if [ $? -ne 0 ]; then
	echo "Couldn't run dnsmasq"
fi

qemu-system-arm -machine $MACHINE -kernel $BLOB_PATH -serial stdio -s -S \
                -net nic,id=mynet0,model=stellaris -net tap,ifname=$HOST_IFACE,script=no,downscript=no
