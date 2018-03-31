# nat_punchthrough

a demo of a simple way of establishing a udp connection through two firewalls

## instalation

enter the following to clone the repo and its submodule then build the project

```
git clone --recurse-submodules https://github.com/isaacmorneau/nat_punchthrough.git
mkdir nat_punchthrough/bin
cd nat_punchthrough/bin
cmake ..
make
./natpunch
```

## usage

simply run this on two networks pointed at each other via -a

```
./bin/natpunch -a <other address>
```

when you see messages get printed out the connection has successfully been established.
