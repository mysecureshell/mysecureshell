Home
====

Synopsis
--------
This directive force the home directory of a connected user. 

Options
-------

========== ========= ============== ============= =======
Name       Default   Values         Since version Context
========== ========= ============== ============= =======
Home       $HOME     home directory 0.1           Default,Group,IpRange,User,VirtualHost 
========== ========= ============== ============= =======

Examples
--------
You can define the default home directory or make a specific home directory like this::

    <Default>
        Home /home/$USER
    </Default>

    <Group secret>
        Home /mnt/nfs/secret/$USER
    </Group>

    <User admin>
        Home /
    </User>

Here all users will have their home directory in ``/home/<username>``, but members of the *secret* group will have their home directory in ``/mnt/nfs/secret/<username>``. To finish, admin user have access to the root directory.
