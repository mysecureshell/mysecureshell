Download
========

.. highlight:: apache

Synopsis
--------
Download tag allows to limit the maximum download bandwidth per connected users.

Options
-------

========== ============= ======== ============= =======
Name       Default       Values   Since version Context
========== ============= ======== ============= =======
Download   0 (unlimited) integer  0.1           Default,Group,IpRange,User,VirtualHost
========== ============= ======== ============= =======

Examples
--------
In this example, all users will be able to download up to a 100k maximum bandwith, while admin users will have 900k at maximum::

    <Default>
        Home /home
        Download 100k
    </Default>

    <User admin>
        Home /home
        Download 900k
    </User>

