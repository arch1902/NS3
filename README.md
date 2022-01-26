### PART 1
```./waf --run "scratch/First --protocol=ns3::<Protocol Name>"```\
For example \
```./waf --run "scratch/First --protocol=ns3::TcpNewReno"```\
```./waf --run "scratch/First --protocol=ns3::TcpHighSpeed"```\
```./waf --run "scratch/First --protocol=ns3::TcpVeno"```\
```./waf --run "scratch/First --protocol=ns3::TcpVegas"```

### PART 2
```./waf --run "scratch/Second --cdr=<Channel Data Rate> --adr=<Application Data Rate>"```\
For example\
```./waf --run "scratch/Second --cdr=2Mbps --adr=2Mbps"```

### PART 3
```./waf --run "scratch/Third_<Configuration Number>"```\
For example\
```./waf --run "scratch/Third_1"```\
```./waf --run "scratch/Third_2"```\
```./waf --run "scratch/Third_3"```






