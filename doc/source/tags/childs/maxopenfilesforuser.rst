MaxOpenFilesForUser
===================

Synopsis
--------
You can limit the number of simultaneous opened files with that tag.

Options
-------

=================== ============= ======== ============= =======
Name                Default       Values   Since version Context
=================== ============= ======== ============= =======
MaxOpenFilesForUser 0 (unlimited) integer  0.3           Default,Group,IpRange,User,VirtualHost
=================== ============= ======== ============= =======

Examples
--------
If you want to limit to 5, the number of simultaneous opened files for a user::

    <Default>
        Home                /home
        MaxOpenFilesForUser 5
    </Default>
