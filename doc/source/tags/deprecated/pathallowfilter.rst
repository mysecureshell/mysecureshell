PathAllowFilter
===============

.. note:: This tag is no more valid since 1.25 version

.. highlight:: apache

Synopsis
--------
With this tag, you can restrict files and folders by using a regular expression.

Options
-------

=============== ========= ======== ============= =======
Name            Default   Values   Since version Context
=============== ========= ======== ============= =======
PathAllowFilter null      regex    1.20 -> 1.25  Default,Group,IpRange,User,VirtualHost
=============== ========= ======== ============= =======

Examples
--------
You can allow filetering only images extensions files like this::

    <Default>
        Home            /home
        PathAllowFilter "^*.(jpg|png|gif)$"
    </Default>
