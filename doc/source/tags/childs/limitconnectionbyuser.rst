LimitConnectionByUser
=====================

.. highlight:: apache

Synopsis
--------
You can limit the number of maximum simultaneous connections per username.

Options
-------

===================== ============= ======== ============= =======
Name                  Default       Values   Since version Context
===================== ============= ======== ============= =======
LimitConnectionByUser 0 (unlimited) integer  0.1           Default,Group,IpRange,User,VirtualHost
===================== ============= ======== ============= =======

Examples
--------
Here we want to limit the total maximum connections to 8. However per username connections, it is restricted to 1 connection::

    <Default>
        Home                  /home
        LimitConnection       8
        LimitConnectionByUser 1
    </Default>

