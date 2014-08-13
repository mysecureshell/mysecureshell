CanRemoveDir
============

.. note:: This tag is no more valid since 1.25 version

Synopsis
--------
To allow users to delete directories, you need to enable this tag. This tag doesn't bypass the default permissions. If you do not have write permissions, you won't be able to remove a directory even is this tag is enable.

Options
-------

============ ========= ======== ============= =======
Name         Default   Values   Since version Context
============ ========= ======== ============= =======
CanRemoveDir false     false    0.9 -> 1.25   Default,Group,IpRange,User,VirtualHost
\                      true
============ ========= ======== ============= =======

Examples
--------
In this example, we allow admins group to remove folders::

    <Default>
        CanRemoveDir false
    </Default>

    <Group admins>
        CanRemoveDir true
    </Group>
