Nagios
======

.. image:: images/logo_nagios.png
    :align: center
    :width: 300px

.. contents:: Topics

.. highlight:: bash

Nagios is a powerful monitoring system that enables organizations to identify and resolve IT infrastructure problems before they affect critical business processes.

We are providing a set of Nagios scripts, compatible with most of the Monitoring tools (Shinken, Naemon, Sensu...).

Presentation and usage
----------------------

If you want to monitor how many users are connected an be alerted if a number of connected users is reached::

    Usage: check_mss_users -w [warning] -c [critical] -p [1|0]
        -h: show help (this message)
        -w: set maximum connected users before warning
        -c: set maximum connected users before critical
        -p: set maximum connected in percentage instead

So for example, if you want to be warned when you're raeching 80 users (warning) and receive a critical alert when you're reaching 100 users::

    $ check_mss_users -w 80 -c 100
    USERS OK - currently connected 0 / 10 |users=0;80;100

You can also decide to use pertencage instead::

    $ check_mss_users -w 80 -c 90 -p 1
    USERS OK - currently connected 0 / 10 |users=0;80;90

This will start alerting at 80% and be critical at 90%.
