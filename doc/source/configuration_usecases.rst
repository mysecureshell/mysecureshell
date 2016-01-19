Use cases
=========

.. contents:: Topics

.. highlight:: apache

To configure MySecureShell, you need to edit the ``/etc/ssh/sftp_config`` file. By default MySecureShell comes with a commented example configuration. This may not match all your needs and you will need to modify it to get it working as expected.

.. note:: Connect as root or use sudo to edit the configuration file.

To help you to understand and see what you can do with MySecureShell, we're covering here some use cases.

Basic usage
-----------
In most of basic situations, you generally want:

* A defined home directory for a user and a set of user (group)
* Limit the bandwidth for the group (users)
* Restrict group to it's home directory
* Have an admin user

Here is a configuration with commented examples::

    <Default>
        # For all users, they will have access to their own home directory
        Home        /home/$USER
    </Default>

    <User admin>
        # This user can have advanced rights to manage MySecureShell from the CLI
        IsAdmin     true
    </User>

    <Group users>
        # Force users to stay in their home directory
        StayAtHome  true
        # Limit their download speed to 128k
        Download    128k
        # Limit their upload speed to 16k
        Upload      16k
    </Group>

Administering multiple websites
-------------------------------
For this use case, let's say you have multiple hosted clients and you have to manage their platforms. You want to:

* Give your clients full rights to manage their website content
* Limit the bandwidth
* Force rights creation
* Limit the number of connexions and idle time
* Restrict their rights on files and folders

Here is a typical configuration::

    <Default>
        # For all users, they will have access to their own home directory
        Home                    /home/$USER
        # Force users to stay in their home directory
        VirtualChroot           true
        # Set global download for the server to 100m
        GlobalDownload          100m
        # Set global upload for the server to 100m
        GlobalUpload            100m
        # Limit user download speed to 1m
        Download                1m
        # Limit user upload speed to 1m
        Upload                  1m
        # Limit 6 users per IP
        LimitConnectionByIP     6
        # We limit a user up to 2 simultaneous connections
        LimitConnectionByUser   2
        # We do not want users to keep forever their idle connection
        IdleTimeOut             10m
        # Force user and group to apache daemon username
        # to avoid rights issues
        ForceUser               www-data
        ForceGroup              www-data
        # We do not want users to be able to set execution files
        MaximumRights           0640 0750
        # We do not want users to be able to change file attributes
        DisableSetAttribute     true
    </Default>

    <VirtualHost www.example.com>
        # Set home directory for this virtualhost
        Home                    /var/www/sites/www.example.com
        # Set dedicated log file
        LogFile                 /var/log/sftp/www.example.com
    </VirtualHost>

    <VirtualHost www.exemple.fr>
        # Set home directory for this virtualhost
        Home                    /var/www/sites/www.exemple.fr
        # Set dedicated log file
        LogFile                 /var/log/sftp/www.example.com
        # Override the maximum number of connection per user
        LimitConnectionByUser   4
    </VirtualHost>

We can see here 2 different VirtualHost which have their own home directory and logs file. They both inherit their configuration from the ``Default`` parent tag. However *www.exemple.fr* virtual host has a different configuration for the maximum of simultaneous connected users.

High restriction access
-----------------------
In a very strict situations, you may want to:

* Restrict your user access to the minimum allowed files (images and pdf only)
* Deny any changes on those files (read only)
* Allow adding new files but in a specific folder (upload)
* Limit to 1 connection per user maximum
* Limit the bandwidth

Let's say the Home folder contents looks like this::

    drwxr-xr-x  2 user group  40 Aug 21 07:14 Download
    drwxrwxrwx  2 user group  40 Aug 21 07:14 Upload

In the Download folder, read only files and folders will be present while in the Upload folder, users will be able to upload anything.
Here is what it should looks like::

    <Default>
        # For all users, they will have access to their own home directory
        Home                    /home/sftp
        # Force users to stay in their home directory
        VirtualChroot           true
        # Set global download for the server to 100m
        GlobalDownload          100m
        # Set global upload for the server to 100m
        GlobalUpload            100m
        # Limit user download speed to 10m
        Download                10m
        # Limit user upload speed to 10m
        Upload                  10m
        # Deny user simultaneous connections
        LimitConnectionByUser   1
        # We do not want users to keep forever their idle connection
        IdleTimeOut             5m
        # We do not want users to be able to modify files once uploaded
        ForceRights             0440 0550
        # To be sure they do not have the permission to delete
        DisableRemoveFile       true
        DisableOverwrite        true
        # We do not want users to be able to change file attributes
        DisableSetAttribute     true
        # Apply filters
        ApplyFileSpec           AllowedExtensions
    </Default>

    <FileSpec AllowedExtensions>
        # Only check against filenames/folder names only
        UseFullPath false
        # we can use multiple deny/allow directives for clarity
        Order AllowDeny
        # Only allow images and pdf extensions
        Allow ".*.(jpg|jpeg|png|gif|raw|psd|pdf)$"
        # Deny anything else
        Deny all
    </FileSpec>

