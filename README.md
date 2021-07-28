# captive-netns-helper

This is a program for Linux that helps you bypass VPN rules in routing table and
custom system DNS resolver and access internet directly via physical interface.
It was written for accessing Wi-Fi captive portals. It comes with a wrapper script
to be easier to use.

Read more [here](https://ch1p.io/gentoo-wireguard-custom-resolver-captive-portal/).

## Installation

```
git clone https://github.com/gch1p/captive-netns-helper
cd captive-netns-helper
mkdir build
cd build
cmake ..
make -j4
sudo make install
```

## Usage
```
captive-portal.sh PROGRAM [ARGUMENTS]
```

For example:
```
captive-portal.sh curl -v https://captive.apple.com
```

## License

MIT