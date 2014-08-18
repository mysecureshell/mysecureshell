ShowLinksAsLinks
================

Synopsis
--------
With ShowLinksAsLinks tag, users symbolic links are not seen as symlink but folders or files. If enabled, connected users simply doesn't know if a file or folders are in fact symbolic links or not.

Options
-------

================ ========= ======== ============= =======
Name             Default   Values   Since version Context
================ ========= ======== ============= =======
ShowLinksAsLinks false     false    0.5           Default,Group,IpRange,User,VirtualHost
\                true
================ ========= ======== ============= =======


Examples
--------
To avoid users to see if folders and files are symbolic links::

    <Default>
        Home             /home
        ShowLinksAsLinks true
    </Default>
