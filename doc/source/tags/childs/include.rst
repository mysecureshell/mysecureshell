Include
=======


Synopsis
--------
With Include tag, you can have additional MySecureShell configuration files to help you to reorganize it if it's becomes to big.

Options
-------

========== ========= ======== ============= =======
Name       Default   Values   Since version Context
========== ========= ======== ============= =======
Include    null      path     0.3           Default,Group,IpRange,User,VirtualHost
========== ========= ======== ============= =======

Examples
--------
To add a new file in the default configuration::

    <Default>
        Home /home
        Include /etc/ssh/mysecureshell/my_sftp_config_file
    </Default>

And then, in ``/etc/ssh/mysecureshell/my_sftp_config_file`` file, you can add configuration content.
