MaximumRights
=============

.. highlight:: apache

Synopsis
--------
Give the maximum allowed rights for new files and folders.

Options
-------

============= ========= ========= ============= =======
Name          Default   Values    Since version Context
============= ========= ========= ============= =======
MaximumRights null      XXXX YYYY 1.20          Default,Group,IpRange,User,VirtualHost
============= ========= ========= ============= =======

* XXXX: numeric files rights
* YYYY: numeric folders rights


Examples
--------
In this example, we do not want users to be able to restrict too much access. So the minimum allowed permissions are ``0640`` for files. And we want to restrict at minimum to ``0750`` to new created folders to the owner and the group only::

    <Default>
        Home /home
        MaximumRights 0640 0750
    </Default>
