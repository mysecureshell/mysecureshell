LogFile
=======

.. highlight:: apache

Synopsis
--------
This tag allows you can separate log files per parent tags.

Options
-------

========== ======================== ======== ============= =======
Name       Default                  Values   Since version Context
========== ======================== ======== ============= =======
LogFile    /var/log/sftp_server.log path     0.95          Default,Group,IpRange,User,VirtualHost
========== ======================== ======== ============= =======

Examples
--------
In this example, we separate the default log location and a virtual host log location::

    <Default>
        LogFile /var/log/sftp_server.log
    </Default>

    <VirtualHost www.example.com>
        LogFile /var/log/sftp/www.example.com.log
    </User>
