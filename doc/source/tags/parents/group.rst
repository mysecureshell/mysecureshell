Group
=====

Synopsis
--------
Group tag is made to add restrictions to a complet set of users defined by a UNIX group. This group has (like users) to be available from your server (PAM, LDAP...).

Options
-------

========== ========= ======== ============= =======
Name       Default   Values   Since version Context
========== ========= ======== ============= =======
Group      null      name     0.1           Override default settings
========== ========= ======== ============= =======

Examples
--------
For example, you can have limited the download speed for all of your users. However, you need your administratorusers in the admins group, to have a better download bandwidth::

    <Default>
        home /home
        # max download speed for the server
        GlobalDownload  200M
        # max download speed for all parent tags
        Download        5M
    </Default>

    <Group admins
        Download        10M
    </Group>

Here all users, will have a maximum of 5M bandwidth and administrators in the admins groups will have up to 10M bandwidth.
