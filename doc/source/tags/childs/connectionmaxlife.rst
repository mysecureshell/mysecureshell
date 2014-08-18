ConnectionMaxLife
=================

Synopsis
--------
Limit the sessions' connection time. By default the value is 0 which means unlimited, there is no deconnection at all, but you can force users to be disconnected. The default unit value is second, but you can change it by adding a suffix to the value (s: seconds, m: minutes, etc...).

Options
-------

================= ========= ======== ============= =======
Name              Default   Values   Since version Context
================= ========= ======== ============= =======
ConnectionMaxLife 0         integer  0.61          Default,Group,IpRange,User,VirtualHost
================= ========= ======== ============= =======

Examples
--------

Here is a basic example where the maximum connection time is 24h, the group admin is unlimited and managers groups inherits of the default settings ::

    <Default>
        Home /home
        ConnectionMaxLife 24h
    </Default>

    <Group admins
        ConnectionMaxLife 0
    </User>

    <Group managers>
        Home /home/managers
    </User>

