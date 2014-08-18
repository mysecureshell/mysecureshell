MaxReadFilesForUser
===================

Synopsis
--------
You can limit the number of simultaneous read files with that tag.

Options
-------

=================== ============= ======== ============= =======
Name                Default       Values   Since version Context
=================== ============= ======== ============= =======
MaxReadFilesForUser 0 (unlimited) integer  0.3           Default,Group,IpRange,User,VirtualHost
=================== ============= ======== ============= =======

Examples
--------
If you want to limit to 5, the number of simultaneous read files for a user::

    <Default>
        Home                /home
        MaxReadFilesForUser 5
    </Default>
