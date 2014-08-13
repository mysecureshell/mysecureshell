DirFakeUser
============

Synopsis
--------
This tag fakes the user owner by visually changing the real user name of files and folders by the user name of the connected user. This can be used for security reasons when you don't want connected users to see real permissions.

Options
-------

============ ========= ======== ============= =======
Name         Default   Values   Since version Context
============ ========= ======== ============= =======
DirFakeUser  false     false    0.1           Default,Group,IpRange,User,VirtualHost
\                      true
============ ========= ======== ============= =======

Examples
--------
Here we want to hide to all users, the files and folders real owner name, but not for the admin user::

    <Default>
        Home /home
        DirFakeUser true
    </Default>

    <User admin>
        DirFakeUser false
    </User>
