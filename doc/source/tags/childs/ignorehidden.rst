IgnoreHidden
============

.. highlight:: apache

Synopsis
--------
Hide or not hidden files or folders (starting with a dot).

Options
-------

============ ========= ======== ============= =======
Name         Default   Values   Since version Context
============ ========= ======== ============= =======
IgnoreHidden false     false    0.1           Default,Group,IpRange,User,VirtualHost
\                      true
============ ========= ======== ============= =======

Examples
--------
If you want to hide all files and folders starting with a dot file (eg: *.DsStore*)::

    <Default>
        Home /home
        IgnoreHidden true
    </Default>
