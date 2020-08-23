import xml.etree.ElementTree as ET
import networkx as nx
import re
import sys
import subprocess
import json
import mdptoolbox, mdptoolbox.example
import numpy as np
from sklearn import preprocessing

class AGParser:

    def __init__(self, agfile):

        self.agfile = agfile
        self.G = nx.DiGraph()
        self.G1 = nx.DiGraph() 
        sys.path.append('cve-search/bin')

    def getCVSS(self, cve):

        proc = subprocess.Popen([" python3 ../cve-search/bin/search.py -c "+ cve +" -o json"], stdout=subprocess.PIPE, shell=True)
        (out, err) = proc.communicate()
        out = json.loads(out.decode('utf-8'))
        complexity = out['access']['complexity']
        prob = 0.0
        if complexity == 'LOW':
           prob = 0.9
        elif complexity == 'MEDIUM':
           prob = 0.6
        else:
           prob = 0.2
         
        res = {'complexity':prob, 'cvss':out['cvss']} 

        return res

    def parseAG(self, agfile):

        tree = ET.parse(self.agfile)
        root = tree.getroot()      
        cve_dict = dict()

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
                        if cve not in cve_dict.keys():
                           cve_dict[cve] = self.getCVSS(cve)

                for succ in list(self.G.successors(node)):
                    if 'attacker' in self.G.nodes[succ]['attrib'] or \
                       'execCode' in self.G.nodes[succ]['attrib'] or \
                       'netAccess' in self.G.nodes[succ]['attrib']:
                       self.G1.add_node(succ)
                       self.G1.nodes[succ]['attrib'] = self.G.nodes[succ]['attrib']
                       n2=succ 
                       self.G1.nodes[succ]['cve'] = cve
                       if cve in cve_dict.keys():
                          self.G1.nodes[succ]['complexity'] = cve_dict[cve]['complexity']
                          self.G1.nodes[succ]['cvss'] = cve_dict[cve]['cvss']
                          

                self.G1.add_edge(n1,n2,cvss=cve)

        #print(cve_dict)       
        #print(self.G1.edges.data())  
        f = open("sample.yaml", "a")
        f.write("sensitive hosts: \n")

        for edge in self.G1.edges():
            cve = str(self.G1[edge[0]][edge[1]]['cvss'])
            if cve in cve_dict.keys():
               f.write(str(edge)+" : "+str(cve_dict[cve]['cvss'])+"\n")
            
            
        f.write("\n")
        f.write("vulnerabilities: \n")
        for edge in self.G1.edges():
            if str(self.G1[edge[0]][edge[1]]['cvss'])!="":
               f.write(" - "+str(self.G1[edge[0]][edge[1]]['cvss'])+"\n")
        
        f.write("os: \n")
        f.write("  - linux \n")
        
        f.write("exploits: \n")
        for edge in self.G1.edges():
            cve = str(self.G1[edge[0]][edge[1]]['cvss'])
            if cve!= "":
               f.write("e_"+cve+": \n")
            if cve in cve_dict.keys():
               f.write("vulnerability: "+cve+"\n")
               f.write("os: linux \n")
               f.write("prob : "+str(cve_dict[cve]['complexity'])+"\n")
               f.write("cost : "+str(cve_dict[cve]['cvss'])+"\n")
        
        f.write("vulnerability_scan_cost: 0.01\n")
        f.write("os_scan_cost: 0.05\n")
        f.write("vuln_configuration: \n")
        for edge in self.G1.edges():
            cve = str(self.G1[edge[0]][edge[1]]['cvss'])
            if cve in cve_dict.keys():
               f.write(str(edge)+" : \n")
               f.write("vulnerability: ["+cve+"]\n")
               f.write("os :linux\n")
        f.write("step_limit: 1000\n")
        state_size = len(self.G1)
        action_size = len(cve_dict)+1

        #S =  [[[0.01 for k in range(state_size)] for j in range(state_size)] for i in range(action_size)]
        #R =  [[[0.01 for k in range(state_size)] for j in range(state_size)] for i in range(action_size)]
        #R = [[0.01]*action_size]*state_size
        S = np.full((action_size, state_size, state_size), 0.01)
        R = np.full((action_size, state_size, state_size), 0.01)
         
         
        # update state table

        for i in range(action_size):
            for j in range(state_size):
                for k in range(state_size):

                 #if there is edge find edge cvss score
                    if self.G1.has_edge(j,k) and self.G1[j][k]['cvss'] in cve_dict.keys():
                       score = 0
                       #print(self.G1[j][k])                         
                       # check all incoming edges to AxSxS' and update score

                       for u,v,cvss in self.G1.edges(k):
                           score = score + self.G1.nodes[v]['complexity']                           
                       if score!=0:
                          S[i][j][k] = self.G1.nodes[k]['complexity']/score
                        
        # Normalize the matrix

        for i in range(action_size):
            S[i] = preprocessing.normalize(S[i])        
            print(S[i])
       
 
        # update reward table
        node_attribs = nx.get_node_attributes(self.G1,'cve')
        print(node_attribs)

        '''for i in range(action_size):
            for j in range(state_size):
                for k in range(state_size):
                   if self.G1.has_node(k):
                      if k in node_attribs.keys():
                         cve = self.G1.nodes[k]['cve']
                         if cve in cve_dict.keys():
                            #print(cve_dict[cve])
                            R[i][j][k]= cve_dict[cve]['cvss']

        print("=============MDP Reward Matrix=============")
        print(R)
        print("=======Value iteration Algorithm Output=====")
        print("=======MDP Generated Policy=================")
        fh = mdptoolbox.mdp.FiniteHorizon(S, R, 0.9, 3) 
        fh.run()
        print(fh.V)
        print(fh.policy) '''

f = AGParser('/home/ubuntu/asap/americano/data/testcases/case1/100_case_AttackGraph.xml')
f.parseAG(f)
