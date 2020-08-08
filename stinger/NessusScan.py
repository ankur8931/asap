import requests
from requests.auth import HTTPBasicAuth
import json
from flask import Flask, request
from flask_restful import Resource, Api, reqparse
from json import dumps
import jsonify
import time
from flask import jsonify
import sys
import argparse

class NessusScan:
 
  def __init__(self):

     print("Initializing Nessus")
     self.username = "admin"
     self.password = "admin"
     self.url = "https://192.168.3.29:8834"
     self.data = {'username':self.username, 'password':self.password}
     self.verify = False
     self.res = requests.post(self.url+'/session',data=self.data, verify=self.verify) 
     self.token = self.res.json().get('token')
     self.headers = {'X-Cookie': 'token=' + self.token, 'content-type': 'application/json'}

  def getScanPolicies(self):
      

      policies =  requests.get(self.url +'/editor/policy/templates',
                                headers=self.headers,verify=self.verify)
      
      if policies.status_code==403:
         return "Unauthorized User"

      elif policies.status_code==200:
         return policies.json()['templates']


  def getScannerState(self):
       
      if(self.res.status_code == 200):
         return self.res.json().get('token') 

              
  def createScan(self, scan_data):
  
      uuid  = ""
      targets = ""

      for ip in scan_data["ipList"]:
          targets+=ip["ip"]+","  
      
      targets = targets[:-1]
      name = scan_data["policy"]

      policies =  requests.get(self.url +'/editor/policy/templates',headers=self.headers,verify=self.verify)

      if policies.status_code==200:
         for policy in policies.json()['templates']:
             if (policy["title"] == scan_data["policy"]):
                uuid = policy["uuid"]
                print(policy)

      payload = {  "uuid" : uuid,
                   "settings" :  {

                         "name" : name,
                         "text_targets" : targets,
                         "launch_now" : True,
                 }
               } 

      payload = json.dumps(payload)  
               
      new_scan = requests.post(self.url + '/scans',data=payload,headers=self.headers,verify=self.verify)

      if new_scan.status_code==200:
         return new_scan.json()['scan']['id']
      else:
         return new_scan.json()['error']
 
    
  def getScanStatus(self, tid):

      status = requests.get(self.url + '/scans/' + str(tid["id"]),headers=self.headers,verify=self.verify)
      
      print(status)
 
      #status = status.json()['info']['status']
      #status = status.json()['info']['status']

      if status=='canceled':  
         return "canceled"

      if status=='paused':
         return "paused"

      if status=='completed':
         return "completed"

      if status=='running':
         time.sleep(2)
         return "running"

      return "scan-id incorrect"


if __name__ == "__main__":


   ns = NessusScan()
   ns.getScannerState()
   ns.getScanPolicies()
     
   scan_data = {"policy": "Advanced Scan", "ipList": [{"ip": "192.168.3.29"}, {"ip": "192.168.3.30"}, {"ip": "192.168.3.31"}]}
   tid = {"id":"21"}

   print(ns.createScan(scan_data))
   #ns.getScanStatus(tid)
