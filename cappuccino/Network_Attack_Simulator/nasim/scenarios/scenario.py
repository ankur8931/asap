import math
from pprint import pprint
import sys
sys.path.insert(1, 'C:/Users/Young_Thug/Desktop/NetworkAttackSimulator/nasim/scenarios')
import utils as u


class Scenario:

    def __init__(self, scenario_dict, name=None):
        self.scenario_dict = scenario_dict
        self.name = name
        self._e_map = None

        # this is used for consistent positioning of
        # host state and obs in state and obs matrices
        self.host_num_map = {}
        for host_num, host_addr in enumerate(self.hosts):
            self.host_num_map[host_addr] = host_num

    @property
    def step_limit(self):
        return self.scenario_dict.get(u.STEP_LIMIT, None)

    @property
    def services(self):
        return self.scenario_dict[u.SERVICES]

    @property
    def num_services(self):
        return len(self.services)

    @property
    def os(self):
        return self.scenario_dict[u.OS]

    @property
    def num_os(self):
        return len(self.os)

    @property
    def exploits(self):
        return self.scenario_dict[u.EXPLOITS]

    @property
    def exploit_map(self):
        """A nested dictionary for all exploits in scenario.

        I.e. {service_name: {
                 os_name: {
                     name: e_name,
                     cost: e_cost,
                     prob: e_prob
                 }
             }
        """
        if self._e_map is None:
            e_map = {}
            for e_name, e_def in self.exploits.items():
                srv_name = e_def[u.EXPLOIT_SERVICE]
                if srv_name not in e_map:
                    e_map[srv_name] = {}
                srv_map = e_map[srv_name]

                os = e_def[u.EXPLOIT_OS]
                if os not in srv_map:
                    srv_map[os] = {
                        "name": e_name,
                        u.EXPLOIT_SERVICE: srv_name,
                        u.EXPLOIT_OS: os,
                        u.EXPLOIT_COST: e_def[u.EXPLOIT_COST],
                        u.EXPLOIT_PROB: e_def[u.EXPLOIT_PROB]
                    }
            self._e_map = e_map
        return self._e_map

    @property
    def subnets(self):
        return self.scenario_dict[u.SUBNETS]

    @property
    def topology(self):
        return self.scenario_dict[u.TOPOLOGY]

    @property
    def sensitive_hosts(self):
        return self.scenario_dict[u.SENSITIVE_HOSTS]

    @property
    def sensitive_addresses(self):
        return list(self.sensitive_hosts.keys())

    @property
    def firewall(self):
        return self.scenario_dict[u.FIREWALL]

    @property
    def hosts(self):
        return self.scenario_dict[u.HOSTS]

    @property
    def address_space(self):
        return list(self.hosts.keys())

    @property
    def service_scan_cost(self):
        return self.scenario_dict[u.SERVICE_SCAN_COST]

    @property
    def os_scan_cost(self):
        return self.scenario_dict[u.OS_SCAN_COST]

    @property
    def subnet_scan_cost(self):
        return self.scenario_dict[u.SUBNET_SCAN_COST]

    @property
    def address_space_bounds(self):
        return len(self.subnets), max(self.subnets)

    @property
    def host_value_bounds(self):
        """The min and max values of host in scenario

        Returns
        -------
        (float, float)
            (min, max) tuple of host values
        """
        min_value = math.inf
        max_value = -math.inf
        for host in self.hosts.values():
            min_value = min(min_value, host.value)
            max_value = max(max_value, host.value)
        return (min_value, max_value)

    @property
    def host_discovery_value_bounds(self):
        """The min and max discovery values of hosts in scenario

        Returns
        -------
        (float, float)
            (min, max) tuple of host values
        """
        min_value = math.inf
        max_value = -math.inf
        for host in self.hosts.values():
            min_value = min(min_value, host.discovery_value)
            max_value = max(max_value, host.discovery_value)
        return (min_value, max_value)

    def display(self):
        pprint(self.scenario_dict)
