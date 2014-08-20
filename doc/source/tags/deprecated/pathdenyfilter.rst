PathDenyFilter
===============

.. note:: This tag is no more valid since 1.25 version

.. highlight:: apache

Synopsis
--------
With this tag, you can restrict files and folders by using a regular expression.

Options
-------

============== ========= ======== ============= =======
Name           Default   Values   Since version Context
============== ========= ======== ============= =======
PathDenyFilter null      regex    0.3 -> 1.25   Default,Group,IpRange,User,VirtualHost
============== ========= ======== ============= =======

Examples
--------
You can deny filetering images extensions files like this::

    <Default>
        Home           /home
        PathDenyFilter "^*.(jpg|png|gif)$"
    </Default>
