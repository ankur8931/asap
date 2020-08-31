
class Host:
    """A single host in the network.

    Note this class is mainly used to store initial scenario data for a host.
    The HostVector class is used to store and track the current state of a
    host (for efficiency and ease of use reasons).
    """

    def __init__(self,
                 address,
                 os,
                 services,
                 value=0.0,
                 discovery_value=0.0,
                 compromised=False,
                 reachable=False,
                 discovered=False):
        """
        Arguments
        ---------
        address : (int, int)
            address of host as (subnet, id)
        os : dict
            A os_name: bool dictionary indicating which OS the host is runinng
        services: dict
            a (service_name, bool) dictionary indicating which services
            are present/absent
        value : float, optional
            value of the host (default=0.0)
        discovery_value : float, optional
            the reward gained for discovering the host (default=0.0)
        compromised : bool, optional
            whether host has been compromised or not (default=False)
        reachable : bool, optional
            whether host is reachable by attacker or not (default=False)
        discovered : bool, optional
            whether host has been reachable discovered by attacker or not
            (default=False)
        """
        self.address = address
        self.os = os
        self.services = services
        self.value = value
        self.discovery_value = discovery_value
        self.compromised = compromised
        self.reachable = reachable
        self.discovered = discovered

    def is_running_service(self, service):
        return self.services[service]

    def is_running_os(self, os):
        return self.os[os]

    def __str__(self):
        output = ["Host: {"]
        output.append(f"\taddress: {self.address}")
        output.append(f"\tcompromised: {self._compromised}")
        output.append(f"\treachable: {self._reachable}")
        output.append(f"\tvalue: {self.value}")
        output.append("\tservices: {")
        for name, val in self.services.items():
            output.append(f"\t\t{name}: {val}")
        output.append("\t}")
        output.append("\tOS: {")
        for os_name, val in self.os.items():
            output.append(f"\t\t{os_name}: {val}")
        output.append("\t}")
        output.append("}")
        return "\n".join(output)

    def __repr__(self):
        return f"Host: {self.address}"
