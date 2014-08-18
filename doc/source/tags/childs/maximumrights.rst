MinimumRights
=============

Synopsis
--------
Give the minimum allowed rights for new files and folders.

Options
-------

============= ========= ========= ============= =======
Name          Default   Values    Since version Context
============= ========= ========= ============= =======
MinimumRights null      XXXX YYYY 1.0           Default,Group,IpRange,User,VirtualHost
============= ========= ========= ============= =======

* XXXX: numeric files rights
* YYYY: numeric folders rights


Examples
--------
In this example, we do not want users to have more than ``0644`` persissions for files (they won't be able to set executable rights). And we want to restrict new created folders to the owner and the group only::

    <Default>
        Home /home
        MinimumRights 0644 0770
    </Default>
