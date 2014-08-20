LimitConnection
===============

.. highlight:: apache

Synopsis
--------
You can limit the number of maximum simultaneous connections.

Options
-------

=============== ============= ======== ============= =======
Name            Default       Values   Since version Context
=============== ============= ======== ============= =======
LimitConnection 0 (unlimited) integer  0.1           Default,Group,IpRange,User,VirtualHost
=============== ============= ======== ============= =======

Examples
--------
Here we want to limit the total maximum connections to 8. However the users members of the *payment* group doesn't have permission to have more than 4 simultaneous connections::

    <Default>
        Home /home
        LimitConnection 8
    </Default>

    <Group payment>
        LimitConnection 4
    <Group>
