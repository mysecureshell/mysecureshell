DisableSymLink
==============

.. highlight:: apache

Synopsis
--------
This tag, deny to the connected user to create symbolic links from files or folders. 

Options
-------

============== ========= ======== ============= =======
Name           Default   Values   Since version Context
============== ========= ======== ============= =======
DisableSymLink false     false    1.20          Default,Group,IpRange,User,VirtualHost
\                        true
============== ========= ======== ============= =======

Examples
--------
In this example, we deny to users in the users group to create symlinks::

    <Default>
        Home /home
    </Default>

    <Group users>
        DisableSymLink true
    </User>

