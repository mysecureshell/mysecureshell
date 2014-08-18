Upload
======

Synopsis
--------
Upload tag allows to limit the maximum upload bandwidth per connected users.

Options
-------

======== ============= ======== ============= =======
Name     Default       Values   Since version Context
======== ============= ======== ============= =======
Upload   0 (unlimited) integer  0.1           Default,Group,IpRange,User,VirtualHost
======== ============= ======== ============= =======

Examples
--------
In this example, all users will be able to upload up to a 100k maximum bandwith, while admin users will have 900k at maximum::

    <Default>
        Home    /home
        Upload  100k
    </Default>

    <User admin>
        Home    /home
        Upload  900k
    </User>

