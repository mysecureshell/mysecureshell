Shell
=====

Synopsis
--------
By default MySecureShell doesn't allow Shell access on SSH connections. But you may want to give to some users Shell access.

Options
-------

========== ========= ======== ============= =======
Name       Default   Values   Since version Context
========== ========= ======== ============= =======
shell      null      path     0.2           Default,Group,IpRange,User,VirtualHost
========== ========= ======== ============= =======

Examples
--------
To restrict a *bash* shell to the `admins` group::

    <Default>
        Home /home
    </Default>

    <Group admins>
        Shell /bin/bash
    </Group>
