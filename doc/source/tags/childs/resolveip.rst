ResolveIP
=========

Synopsis
--------
You can have DNS name in your logs instead of IP addresses if you enable ResolveIP tag.

Options
-------

========== ========= ======== ============= =======
Name       Default   Values   Since version Context
========== ========= ======== ============= =======
ResolveIP  false     false    0.3           Default,Group,IpRange,User,VirtualHost
\                    true
========== ========= ======== ============= =======

Examples
--------
Here I want to resolve IP addresses for all connections::

    <Default>
        Home /home
        ResolveIP true
    </Default>
