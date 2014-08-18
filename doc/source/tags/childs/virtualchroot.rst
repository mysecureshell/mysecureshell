VirtualChroot
=============

Synopsis
--------
This tag permit to add security to :doc:`StayAtHome tag<stayathome>`. It chroot the user in his home directory. So the user won't be able to know on which folder his home directory is located on the server. His home directory will be shown as `/`.

Options
-------

============= ========= ======== ============= =======
Name          Default   Values   Since version Context
============= ========= ======== ============= =======
VirtualChroot false     false    0.1           Default,Group,IpRange,User,VirtualHost
\                       true
============= ========= ======== ============= =======

Examples
--------
In this example, we enabled the `VirtualChroot` for all users::

    <Default>
        Home          /home
        VirtualChroot true
    </Default>
