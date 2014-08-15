DisableReadDir
==============

Synopsis
--------
This tag, deny to the connected user to read folders content.

Options
-------

============== ========= ======== ============= =======
Name           Default   Values   Since version Context
============== ========= ======== ============= =======
DisableReadDir false     false    1.20          Default,Group,IpRange,User,VirtualHost
\                        true
============== ========= ======== ============= =======

Examples
--------
In this example, we deny to users in the users group to read directories content::

    <Default>
        Home /home
    </Default>

    <Group users>
        DisableReadDir true
    </User>

