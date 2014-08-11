User
====

Synopsis
--------
User's tag can only affects one user.

Options
-------

========== ========= ======== ============= =======
Name       Default   Values   Since version Context
========== ========= ======== ============= =======
User       null      name     0.1           Override default settings
========== ========= ======== ============= =======

Examples
--------
In this example, you will see how to give to a user specific rights::

    <Default>
        Home /home/$USER
    </Default>

    <User admin
        Home /home/
    </User>

You can see here all users will have their home in ``/home/$USER`` while admin user will have his home directory in ``/home``.
