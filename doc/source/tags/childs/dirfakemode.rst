DirFakeMode
===========

Synopsis
--------
This tag fakes the global permissions by visually changing the real rights of files and folders by the defined ones. This can be used for security reasons when you don't want connected users to see real permissions.

Options
-------

============ ========= ======== ============= =======
Name         Default   Values   Since version Context
============ ========= ======== ============= =======
DirFakeMode  false     false    0.1           Default,Group,IpRange,User,VirtualHost
\                      true
============ ========= ======== ============= =======

Examples
--------
Here we want to hide all public users (by using the public group) the files and folders::

    <Default>
        Home /home
    </Default>

    <Group public>
        DirFakeMode 0777
    </Group>

