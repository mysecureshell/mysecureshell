ByPassGlobalUpload
====================

.. highlight:: apache

Synopsis
--------
This tag is made to override the GlobalUpload tag.

Options
-------

================== ========= ======== ============= =======
Name               Default   Values   Since version Context
================== ========= ======== ============= =======
ByPassGlobalUpload false     false    0.5           Default,Group,IpRange,User,VirtualHost
\                            true
================== ========= ======== ============= =======

Examples
--------
In this example, the user admin won't be limited by the GlobalUpload limitation set on the default tag::

    <Default>
        home /home
        GlobalUpload 100k
    </Default>

    <User admin>
        ByPassGlobalUpload true
    </User>

Admin user will have full upload speed.
