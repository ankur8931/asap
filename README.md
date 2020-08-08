# asap
## Autonomous Security Analysis and Penetration Testing
stinger - port scan and vulnerablity scan APIs 
americano - generated attack graph files. Need a local setup of mulval tool
cappuccino - code files for MDP solver. Need pymdptoolbox and pymetasploit framework

## Source code description
```
stinger/PortScan.py - Python based nmap port scanner for Stinger's network discovery
   
cappuccino/AGParser.py - Python parser for extracting attack graph information for creating MDP matrix

stinger/NessusScan.py - Automated Vulnerablity scan APIs. APIs disabled in current version 
of nessus

env.sh - Source files for setting up americano attack graph generation modules
```           

## Additional Dependencies
```
graphviz - Graph Plotting
mongodb - cvesearch information storage
mysql - attack graph data storage
XSB - Prolog rules for the generation of attack graph
pymdptoolbox - MDP solver for MDP formed using attack graph
```

