IpRange
=======

.. highlight:: apache

Synopsis
--------
With IpRange, you can define limitations to an IP address or a range of IP adresses.

Options
-------

========== ========= ================= ============= =======
Name       Default   Values            Since version Context
========== ========= ================= ============= =======
IpRange    null      IP/CIDR-IP/CIDR   0.4           Override default settings
\                    IP/CIDR                                
========== ========= ================= ============= =======

Examples
--------
In the first example, you may want to change download speed if users are coming from your local network, so you're adding your range of IPs::

    <Default>
        GlobalDownload  500M
        Download        100M
    </Default>

    <IpRange 192.168.0.100/24-192.168.0.200/24>
        Download        300M
    </IpRange>

Here, my external users have 100M maximum download bandwith and my local users are limited to 300M. If you now want to have a super user with unrestricted speed, you do it like that::

    <IpRange 192.168.0.1/24>
        Download        0
    </IpRange>

This IP address won't be limited and will have the maximum bandwidth.
