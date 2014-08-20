MaxWriteFilesForUser
====================

.. highlight:: apache

Synopsis
--------
You can limit the number of simultaneous write files with that tag.

Options
-------

==================== ============= ======== ============= =======
Name                 Default       Values   Since version Context
==================== ============= ======== ============= =======
MaxWriteFilesForUser 0 (unlimited) integer  0.3           Default,Group,IpRange,User,VirtualHost
==================== ============= ======== ============= =======

Examples
--------
If you want to limit to 5, the number of simultaneous write files for a user::

    <Default>
        Home                 /home
        MaxWriteFilesForUser 5
    </Default>
