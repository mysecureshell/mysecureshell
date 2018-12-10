DisableStatsFs
==============

.. highlight:: apache

Synopsis
--------
This tag, deny to the connected user to get volume informations.

Options
-------

============== ========= ======== ============= =======
Name          Default   Values   Since version Context
============== ========= ======== ============= =======
DisableStatsFs false     false    2.00          Default,Group,IpRange,User,VirtualHost
\                       true
============== ========= ======== ============= =======

Examples
--------
In this example, we deny to users in the users group to get volume informations::

    <Default>
        Home /home
    </Default>

    <Group users>
        DisableStatsFs true
    </User>

