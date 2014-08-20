ByPassGlobalDownload
====================

.. highlight:: apache

Synopsis
--------
This tag is made to override the GlobalDownload tag.

Options
-------

==================== ========= ======== ============= =======
Name                 Default   Values   Since version Context
==================== ========= ======== ============= =======
ByPassGlobalDownload false     false    0.5           Default,Group,IpRange,User,VirtualHost
\                              true
==================== ========= ======== ============= =======

Examples
--------
In this example, the user admin won't be limited by the GlobalDownload limitation set on the default tag::

    <Default>
        home /home
        GlobalDownload 100k
    </Default>

    <User admin>
        ByPassGlobalDownload true
    </User>

Admin user will have full download speed here.
