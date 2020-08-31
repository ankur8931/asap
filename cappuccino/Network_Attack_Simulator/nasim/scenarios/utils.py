import os.path as osp


SCENARIO_DIR = osp.dirname(osp.abspath(__file__))

# default subnet address for internet
INTERNET = 0

# scenario property keys
SUBNETS = "subnets"
TOPOLOGY = "topology"
SENSITIVE_HOSTS = "sensitive_hosts"
SERVICES = "services"
OS = "os"
EXPLOITS = "exploits"
SERVICE_SCAN_COST = "service_scan_cost"
OS_SCAN_COST = "os_scan_cost"
SUBNET_SCAN_COST = "subnet_scan_cost"
HOST_CONFIGS = "host_configurations"
FIREWALL = "firewall"
HOSTS = "host"
STEP_LIMIT = "step_limit"

# scenario exploit keys
EXPLOIT_SERVICE = "service"
EXPLOIT_OS = "os"
EXPLOIT_PROB = "prob"
EXPLOIT_COST = "cost"

# host configuration keys
HOST_SERVICES = "services"
HOST_OS = "os"
