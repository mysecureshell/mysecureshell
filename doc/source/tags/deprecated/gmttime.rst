GMMTTime
========

.. note:: This tag is no more valid since 1.25 version

Synopsis
--------
Adjust the clock time in the log file

Options
-------

========== ========= =================== ============= =======
Name       Default   Values              Since version Context
========== ========= =================== ============= =======
Charset    null      x(+,-)              0.8 -> 1.25   Default
                     y(-12,+12 hour GMT)
========== ========= =================== ============= =======

Examples
--------
In this example, we're changing the GMTTime::

    <Default>
        Home /home
        GMTTime +1
    </Default>
