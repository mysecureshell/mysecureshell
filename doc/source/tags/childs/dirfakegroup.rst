DirFakeGroup
============

Synopsis
--------
This tag fakes the group owner by visually changing the real group name of files and folders by the group of the connected user. This can be used for security reasons when you don't want connected users to see real permissions.

Options
-------

============ ========= ======== ============= =======
Name         Default   Values   Since version Context
============ ========= ======== ============= =======
DirFakeGroup false     false    0.1           Default,Group,IpRange,User,VirtualHost
\                      true
============ ========= ======== ============= =======

Examples
--------
Here we want to hide all users the files and folders real groups, but not for the admin user::

    <Default>
        Home /home
        DirFakeGroup true
    </Default>

    <User admin>
        DirFakeGroup false
    </User>
