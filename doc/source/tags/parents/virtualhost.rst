VirtualHost
===========

Synopsis
--------
Like web servers, you can define limitations depending on the virtualhost name you're targetting.

Options
-------

=========== ========= ========= ============= =======
Name        Default   Values    Since version Context
=========== ========= ========= ============= =======
VirtualHost null      name:port 0.6           Override default settings
=========== ========= ========= ============= =======

Examples
--------
In this example you will see how to change a home directory depending on the used VirtualHost::

    <Default>
        home /home
    </Default>

    <VirtualHost en.mysite.com:22>
        home /home/en.mysite.com
    </VirtualHost>

    <VirtualHost fr.mysite.com:22>
        home /home/fr.mysite.com
    </VirtualHost>

If you try to connect on fr.mysite.com, you will be redirected to ``/home/fr.mysite.com`` folder.
If you try to connect on en.mysite.com, you will be redirected to ``/home/en.mysite.com`` folder.
