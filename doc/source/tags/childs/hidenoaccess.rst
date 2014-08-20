HideNoAccess
============

.. highlight:: apache

Synopsis
--------
This tags hides files and folders denied from the connected user.

Options
-------

============ ========= ======== ============= =======
Name         Default   Values   Since version Context
============ ========= ======== ============= =======
HideNoAccess false     false    0.3           Default,Group,IpRange,User,VirtualHost
\                      true
============ ========= ======== ============= =======

Examples
--------
If you want to automatically hide unauthorized files and folders::

    <Default>
        home /home
        HideNoAccess true
    </Default>
