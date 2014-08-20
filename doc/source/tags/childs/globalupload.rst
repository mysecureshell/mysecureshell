GlobalUpload
============

.. highlight:: apache

Synopsis
--------
Global Upload tag allows to limit the maximum upload bandwidth for the overall server.

Options
-------

============ ============= ======== ============= =======
Name         Default       Values   Since version Context
============ ============= ======== ============= =======
GlobalUpload 0 (unlimited) integer  0.1           Default
============ ============= ======== ============= =======

Examples
--------
In this example, all users will be able to upload up to a 100k maximum bandwith and the total maximum upload bandwidth allowed for the server is 1M::

    <Default>
        Home /home
        GlobalUpload 1M
        Upload 100k
    </Default>

