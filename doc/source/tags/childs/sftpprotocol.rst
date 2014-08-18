SftpProtocol
============

Synopsis
--------
You can force a specific version of the SFTP protocol to enable or disable some features. To know more about them you can consult those websites:

* `<http://www.greenend.org.uk/rjk/sftp/sftpversions.html>`_
* `<http://tools.ietf.org/html/draft-ietf-secsh-filexfer-13>`_

Options
-------

============ ========= ======== ============= =======
Name         Default   Values   Since version Context
============ ========= ======== ============= =======
SftpProtocol 4         integer  0.6           Default,Group,IpRange,User,VirtualHost
============ ========= ======== ============= =======

Examples
--------
In this example, we want to force `oldclients` group to use SFTP protocol in version 3. All other users will be in version 4 of the protocol::

    <Default>
        Home         /home
        SftpProtocol 4
    </Default>

    <Group oldclients>
        SftpProtocol 3
    </Group>
