ExpireDate
==========

.. highlight:: apache

Synopsis
--------
This tag permit to set an expiration date time on a parent tag, which means users won't be allowed to connecte after the defined date and time. The date and time should be in strftime format (man 3 strftime). You can test date format using date command:

.. code-block:: bash

 $ echo $(date '+%Y-%m-%d %H:%M:%S')
 2014-08-15 15:21:19

Options
-------

========== ================ ================= ============= =======
Name       Default          Values            Since version Context
========== ================ ================= ============= =======
ExpireDate null (unlimited) %Y-%m-%d %H:%M:%S 1.0           Default,Group,IpRange,User,VirtualHost
========== ================ ================= ============= =======

Examples
--------
In this example, we want to set a group of users expiration::

    <Default>
        Home /home
    </Default>

    <Group users>
        ExpireDate "2015-12-25 00:00:00"
    </User>

The users group will expire on 25 december 2015 at midnight.
