#!/bin/bash

if [ $EUID -eq 0 ]; then
    echo "error: this script should not be launched as root"
    exit 1
fi

if [ $# -eq 0 ]; then
    echo "error: no command specified"
    exit 1
fi

export $(dhcpcd -U $IFACE)
if [ -z "$domain_name_servers" ]; then
    echo "error: \$domain_name_servers variable not found"
    exit 1
fi

IFACE=wlp3s0
ENV=
for var in DISPLAY HOME PWD EDITOR USER XAUTHORITY LANG DBUS_SESSION_BUS_ADDRESS; do
    value="${!var}"
    if [ ! -z "$value" ]; then
        ENV="$ENV --env $var=$value"
    fi
done

_doas="doas"
if ! command -v doas &>/dev/null; then
    _doas="sudo"
fi

$_doas captive-netns-helper \
    --nameserver $domain_name_servers \
    --ns-file /run/netns/captive \
    --uid $(id -u) --gid $(id -g) $ENV "$@"