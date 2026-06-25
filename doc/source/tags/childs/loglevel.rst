LogLevel
========

.. highlight:: apache

Synopsis
--------
This tag allows you can modifiy log level per parent tags.

Options
-------

========== ========= ======================= ============= =======
Name       Default   Values                  Since version Context
========== ========= ======================= ============= =======
LogLevel   6 (all)   0 (no log)              2.01          Default,Group,IpRange,User,VirtualHost
\                    1 (show connexion)
\                    2 (show transfert)
\                    3 (show error)
\                    4 (show warning)
\                    5 (show info)
\                    6 (show debug)
========== ========= ======================= ============= =======

Examples
--------
In this example, we modify the default log level and a virtual host log location::

    <Default>
        LogLevel 7
    </Default>

    <VirtualHost www.example.com>
        LogLevel 3
    </User>
