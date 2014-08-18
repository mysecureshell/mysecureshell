IsAdmin
=======

Synopsis
--------
IsAdmin tag grants to a user or a set of users, admins permissions. That means they can administrate MySecureShell throught ``sftp-admin`` CLI or with MySecureShell GUI.

Options
-------

========== ========= ======== ============= =======
Name       Default   Values   Since version Context
========== ========= ======== ============= =======
IsAdmin    false     false    0.7           Default,Group,IpRange,User,VirtualHost
\                    true
========== ========= ======== ============= =======

Examples
--------
If you want to grant to *admins* group the administrative rights::

    <Default>
        Home /home
    </Default>

    <Group admins>
        IsAdmin true
    </Group>
