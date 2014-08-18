IdleTimeOut
===========

Synopsis
--------
This tag disconnects users when the maximum idle time has been reached. The default unit value is second, but you can change it by adding a suffix to the value (s: seconds, m: minutes, etc...).

Options
-------

Name        Default       Values   Since version Context
=========== ============= ======== ============= =======
IdleTimeOut 0 (unlimited) integer  0.1           Default,Group,IpRange,User,VirtualHost

Examples
--------
If you want to free connexions for idle users after a certain amount of time, you can set an idle timeout value like this::

    <Default>
        Home /home
        IdleTimeOut 5m
    </Default>

