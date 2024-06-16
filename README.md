## DNS Proxy Server

#### Overview
This is a DNS proxy server equipped with a domain blacklist feature to filter unwanted host name resolutions.

#### Configuration
- The proxy server reads its parameters from the configuration file located at `./config/config.ini`.

  ###### Setting up `config.ini`:
  - `ip` (Required): Upstream DNS server IP address.
  - `domain`: List of domain names to block (blacklist).
  - `response` (Required): Proxy's response for blacklisted domains.
  - `resolv_ip` (Required): Proxy's local DNS address (should match your nameserver in `resolv.conf`). (You can use the 'dig' command to determine this.)

#### Operation
- This DNS proxy server uses UDP protocol to handle client requests and communicate with the upstream DNS server.
- When a client request's domain name is not found in the blacklist, the proxy forwards the request to the upstream DNS server, awaits a response, and then relays it back to the client.
- If a domain name is found in the blacklist, the proxy server responds with the predefined response from the configuration file.

#### Usage
#### Before starting the proxy, ensure that any processes using port 53 on your machine are disabled using `sudo lsof -i :53` or similar commands.
1. Clone the repository locally:
   - `git clone https://github.com/Myrrrca/dns-proxy`
2. Configure your `config.ini` file following the guidelines under "Configuration" and save it.
3. Use the Makefile commands to build the executable:
   - `make proxy` (to build)
   - `make clean` (to remove the executable)
4. Once built successfully, run the following command:
   - `sudo ./proxy`

#### Additional Information
- This DNS proxy server does not utilize any third-party libraries or code except for the "inih" library for parsing `.ini` files (https://github.com/benhoyt/inih). This approach was chosen to gain a deeper understanding of DNS query and response mechanisms.
- Since this proxy communicates exclusively via UDP packets, it does not handle TCP protocol redirection for requests or responses exceeding 512 bytes (in accordance with RFC 1035). This limitation may affect the server's performance under such conditions, which is outside the scope of this application's functionality.
- The implementation of this proxy does not include multiplexing.
- Please report any bugs or issues on the project's GitHub Issues page (https://github.com/Myrrrca/dns-proxy), providing details, including the domain name if applicable.

