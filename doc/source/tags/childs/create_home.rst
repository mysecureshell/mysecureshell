CreateHome
==========

Synopsis
--------
Create user's home directory if it doesn't exists.

Options
-------

========== ========= ======== ============= =======
Name       Default   Values   Since version Context
========== ========= ======== ============= =======
CreateHome false     false    1.20          Default,Group,IpRange,User,VirtualHost
\                    true
========== ========= ======== ============= =======

Examples
--------
In this example, we want home directory to be created for each users::

    <Default>
        Home /home
        CreateHome true
    </Default>

