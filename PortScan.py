import nmap

class PortScan:
  
   def __init__(self):

      print("Scanning Network")

   def scanTarget(self, ip):

      nm = nmap.PortScanner()
      nm.scan(hosts=ip)
      serv_list = list()

      for proto in nm[ip].all_protocols():
          lport = nm[ip][proto].keys()

          for port in lport:

              serv_list.append({"port":port,"state":nm[ip][proto][port]['state'],"protocol":proto,"service_name":nm[ip][proto][port]['product'], "cpe":nm[ip][proto][port]['cpe']})
      print(serv_list)

      return serv_list

if __name__ == "__main__":
   ps = PortScan()
   ps.scanTarget('192.168.3.36')

