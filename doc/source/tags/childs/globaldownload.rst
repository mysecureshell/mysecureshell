GlobalDownload
==============

.. highlight:: apache

Synopsis
--------
Global Download tag allows to limit the maximum download bandwidth for the overall server.

Options
-------

============== ============= ======== ============= =======
Name           Default       Values   Since version Context
============== ============= ======== ============= =======
GlobalDownload 0 (unlimited) integer  0.1           Default
============== ============= ======== ============= =======

Examples
--------
In this example, all users will be able to download up to a 100k maximum bandwith and the total maximum bandwidth allowed for the server is 1M::

    <Default>
        Home /home
        GlobalDownload 1M
        Download 100k
    </Default>

