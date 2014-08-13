DisableAccount
==============

Synopsis
--------
Here is an easy way to disable accounts for a user or a set of users. This can be useful if you need to disable accounts quickly. **Only new connections will be deny, current open ones won't be automatically closed**.

Options
-------

============== ========= ======== ============= =======
Name           Default   Values   Since version Context
============== ========= ======== ============= =======
DisableAccount false     false    0.7           Default,Group,IpRange,User,VirtualHost
\                        true
============== ========= ======== ============= =======

Examples
--------
In this example, we disable a set of users by disabling the group accounts::

    <Default>
        Home /home
    </Default>

    <Group users>
        DisableAccount false
    </User>

