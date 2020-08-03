import xml.etree.ElementTree as ET
import networkx as nx
import re
class AGParser:

    def __init__(self, agfile):
        self.agfile = agfile
        self.G = nx.DiGraph()
        self.G1 = nx.DiGraph()

    def parseAG(self, agfile):
        tree = ET.parse(self.agfile)
        root = tree.getroot()      

        # Parse over arcs and create a directed graph
        for child in root:
            for gc in child:
                if(gc.tag == 'arc'):
                   self.G.add_edge(int(gc[1].text), int(gc[0].text))
                                 
        # Parse over vertices and check if post-conditions 
        # are connected by RULE node 
        for child in root:
            for gc in child:
                if(gc.tag == 'vertex'):
                   self.G.nodes[int(gc[0].text)]['attrib'] = gc[1].text 
                   
        for node in self.G.nodes:
            if 'RULE' in self.G.nodes[node]['attrib']:
                n1=""
                n2 =""
                cve= ""
                for pred in list(self.G.predecessors(node)):
                    if 'attacker' in self.G.nodes[pred]['attrib'] or \
                       'execCode' in self.G.nodes[pred]['attrib'] or \
                       'netAccess' in self.G.nodes[pred]['attrib']:
                       self.G1.add_node(pred)
                       self.G1.nodes[pred]['attrib'] = self.G.nodes[pred]['attrib']
                       n1=pred
                    if 'vulExists' in self.G.nodes[pred]['attrib']:
                        cve = self.G.nodes[pred]['attrib'].split(',')[1]
                        cve = cve.strip('\'')
                        print(cve)
                for succ in list(self.G.successors(node)):
                    if 'attacker' in self.G.nodes[succ]['attrib'] or \
                       'execCode' in self.G.nodes[succ]['attrib'] or \
                       'netAccess' in self.G.nodes[succ]['attrib']:
                       self.G1.add_node(succ)
                       self.G1.nodes[succ]['attrib'] = self.G.nodes[succ]['attrib']
                       n2=succ 
                self.G1.add_edge(n1,n2,cvss=cve)
               
        print(self.G1.edges.data())   
f = AGParser('data/testcases/case1/100_case_AttackGraph.xml')
f.parseAG(f)
