DefaultRights
=============

.. highlight:: apache

Synopsis
--------
Set default rights on new created (or uploaded) files and folders. You have to set `UNIX numeric notation <http://en.wikipedia.org/wiki/Filesystem_permissions>`_ permissions for files and folders.

Options
-------

============= ====================== ========= ============= =======
Name          Default                Values    Since version Context
============= ====================== ========= ============= =======
DefaultRights file and folder rights XXXX YYYY 0.3           Default,Group,IpRange,User,VirtualHost
============= ====================== ========= ============= =======

* XXXX: numeric files rights
* YYYY: numeric folders rights

Examples
--------
In this example, default options are set to give full rights to any new files and folders (0777). But we want to be sure that secret groups keeps their files and folders owned by themselfs::

    <Default>
        Home /home
        DefaultRights 0777 0777
    </Default>

    <Group secret>
        Home /home/secret
        DefaultRights 0700 0700
    </User>
