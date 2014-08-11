Default
=======

Synopsis
--------
Default tag is mandatory to apply a default configuration to all others parents' tags and. It is also used if you want to apply a configuration for all of your users, even if you do not declare other parent tags.

Options
-------

========== ========= ======== ============= =======
Name       Default   Values   Since version Context
========== ========= ======== ============= =======
Default    null      null     0.4           Mandatory
========== ========= ======== ============= =======

Examples
--------
Here is a Default tag defined with an Upload limit set to 100k::

    <Default>
        Upload 100k
    </Default>

