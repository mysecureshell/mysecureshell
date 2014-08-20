LogSyslog
=========

.. highlight:: apache

Synopsis
--------
Write logs to Syslog. If you want to know more on Syslog, you can follow the `link <http://en.wikipedia.org/wiki/Syslog>`_.

Options
-------

========== ======= ====== ============= =======
Name       Default Values Since version Context
========== ======= ====== ============= =======
LogSyslog  false   false  1.33          Default,Group,IpRange,User,VirtualHost
\                  true
========== ======= ====== ============= =======

Examples
--------
If you want to enable syslog loging::

    <Default>
        Home      /home
        LogSyslog true
    </Default>
