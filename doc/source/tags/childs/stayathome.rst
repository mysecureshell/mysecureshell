StayAtHome
==========

Synopsis
--------
The StayAtHome tag force the user to stay in his home directory. He won't be able to move to a higher hierarchy level. For example, a user getting his home directory in `/home/user` won't be able to got to `/home` or `/` for example.

Options
-------

========== ========= ======== ============= =======
Name       Default   Values   Since version Context
========== ========= ======== ============= =======
StayAtHome false     false    0.4           Default,Group,IpRange,User,VirtualHost
\                    true
========== ========= ======== ============= =======

Examples
--------
In this example, we want to be sure all users will stay in their home directory with sub directories::

    <Default>
        Home         /home
        StayAtHome   true
    </Default>
