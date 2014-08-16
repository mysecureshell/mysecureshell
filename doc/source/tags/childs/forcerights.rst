ForceRights
===========

Synopsis
--------
You can force the default permissions when a connected user create files and folders. That mean when a user will connect, when files or folders are created, the default specified rights will be applyed.

Options
-------

=========== ========= ========== ============= =======
Name        Default   Values     Since version Context
=========== ========= ========== ============= =======
ForceRights null      XXXX YYYY  1.20          Default,Group,IpRange,User,VirtualHost
=========== ========= ========== ============= =======

* XXXX: numeric files rights
* YYYY: numeric folders rights

Examples
--------
You can force files and folders permissions like this::

    <Default>
        Home /home
    </Default>

    <User username>
        ForceRights 0640 0750
    </User>

Here the user *username* will be forced to create 0640 files rights and 0750 folders rights.
