LimitConnectionByIp
===================

Synopsis
--------
You can limit the number of maximum simultaneous connections per incoming IP address.

Options
-------

=================== ============= ======== ============= =======
Name                Default       Values   Since version Context
=================== ============= ======== ============= =======
LimitConnectionByIp 0 (unlimited) integer  0.1           Default,Group,IpRange,User,VirtualHost
=================== ============= ======== ============= =======

Examples
--------
Here we want to limit the total maximum connections to 8. However per connections IP address, it is restricted to 1 connection::

    <Default>
        Home                /home
        LimitConnection     8
        LimitConnectionByIp 1
    </Default>

