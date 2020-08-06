# asap
## Autonomous Security Analysis and Penetration Testing

## Source code description
```
PortScan.py - Python based nmap port scanner for Stinger's network discovery
   
AGParser.py - Python parser for extracting attack graph information for creating MDP matrix

NessusScan.py - Automated Vulnerablity scan APIs. APIs disabled in current version 
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

