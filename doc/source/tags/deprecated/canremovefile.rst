CanRemoveFile
=============

.. note:: This tag is no more valid since 1.25 version

Synopsis
--------
To allow users to delete files, you need to enable this tag. This tag doesn't bypass the default permissions. If you do not have write permissions, you won't be able to remove a file even is this tag is enable.

Options
-------

============= ========= ======== ============= =======
Name          Default   Values   Since version Context
============= ========= ======== ============= =======
CanRemoveFile false     false    0.9 -> 1.25   Default,Group,IpRange,User,VirtualHost
\                       true
============= ========= ======== ============= =======

Examples
--------
In this example, we allow admins group to remove files::

    <Default>
        CanRemoveFile false
    </Default>

    <Group admins>
        CanRemoveFile true
    </Group>
