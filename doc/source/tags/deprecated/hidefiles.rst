HideFiles
=========

.. note:: This tag is no more valid since 1.25 version

.. highlight:: apache

Synopsis
--------
With HideFiles, you can hide files and directories using regular expressions.

Options
-------

========= ========= ======== ============= =======
Name      Default   Values   Since version Context
========= ========= ======== ============= =======
HideFiles regex     null     1.0 -> 1.25   Default,Group,IpRange,User,VirtualHost
========= ========= ======== ============= =======

Examples
--------
In this example we are hiding *lost+found* or *public_html* files and folders::

    <Default>
        Home /home
        HideFiles "^(lost+found|public_html)$"
    </Default>
