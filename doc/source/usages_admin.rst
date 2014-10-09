For Administrators
==================

.. contents:: Topics

.. highlight:: bash

As an administrator, you have to tasks to know to manage your users and validate the configuration.

Configuration validator
-----------------------

Let's say you've made a complex MySecureShell configuration and you want to validate the configuration of some users. MySecureShell can natively perform those check for you and gives you the configuration related to a user::

    $ MySecureShell --configtest
    --- invite ---
    Home = /home/invite
    ByPassGlobalDownload = false
    ByPassGlobalUpload = false
    GlobalDownload = 5242880 bytes/s
    GlobalUpload = 0 bytes/s
    Download = 0 bytes/s
    Upload = 0 bytes/s
    StayAtHome = true
    VirtualChroot = true
    LimitConnection = 3
    LimitConnectionByUser = 3
    LimitConnectionByIP = 1
    IdleTimeOut = 300s
    ResolveIP = false
    DirFakeUser = false
    DirFakeGroup = true
    DirFakeMode = 0000
    HideFiles = {nothing to hide}
    HideNoAccess = true
    MaxOpenFilesForUser = 0
    MaxReadFilesForUser = 0
    MaxWriteFilesForUser = 0
    PathDenyFilter = (null)
    Shell = {no shell}
    ShowLinksAsLinks = false
    DefaultRights = 0000 0000
    ConnectionMaxLife = 0s
    DisableAccount = false
    IsAdmin = false
    Config is valid.

You can also enable verbose mode with ``-v`` argument. As you can see here, the message on the last line indicates the configuration is valid, with means there is no syntax errors.

Updating configuration
----------------------

With MySecureShell, **you do not need to reload the service when you make a new or update the configuration**. Simply add your new parameters and **once you write changes to disks, it will be applyed**.

Service status
--------------

By default, MySecureShell doesn't require a daemon as the configuration is reread each time a user connects to the server. But you may want to temporary stop the service, start it or getting the status::

    service mysecureshell stop

It will in fact add a lock file to the filesystem to inform MySecureShell to deny any new connections. This lock file is released once the service is started.

All available options are:

======================= ===========
Options                 Description
======================= ===========
stop / shutdown         Deny new SFTP connections and keep current connected users
start / active          Allow new SFTP connections
fullstop                Deny new SFTP connexions and kill current running (clean memory as well)
restart / force-reload  Make a fullstop and start
status                  Give the current status (up/down)
======================= ===========

.. note:: The ``service`` command uses ``sftp-state`` binary. You can have a look at it if you do not want to use the ``service`` command.

Who is connected
----------------

You can know how many users are connect, who, the used global bandwidth, per user bandwidth usage, the status and other usefull information with the ``sftp-who`` command::

    > sftp-who
    --- 1 / 10 clients ---
    Global used bandwith : 0 bytes/s / 0 bytes/s
    PID: 3389   Name: mssuser   IP: 
        Home: /home/mssuser
        Status: idle    Path: /
        File: 
        Connected: 2014/08/19 15:38:27 [since 10s]
        Speed: Download: 0 bytes/s [5.00 kbytes/s]  Upload: 0 bytes/s [unlimited]
        Total: Download: 1398 bytes   Upload: 141 bytes

You also have ``--while`` option to automatically refresh the information with a desired time in seconds::

    > sftp-who --while 2

This will force ``sftp-who`` to refresh every 2 seconds.

MySecureShell control remotely
------------------------------

You can remotely control MySecureShell and make actions on users, server state and more. Simply connect on SSH with an admin account and you will have those features::

    > sftp-admin admin@sftp.example.com
    admin@sftp.example.com's password: 
    > help
    Usage:
         kill [0 or PID] : kill user with PID or 0 to kill all users
         list : list online users
         log [x bytes] : show last x bytes of log
         quit : quit program
         server [start or stop] : start or stop server

Check MySecureShell installation
--------------------------------

You can check MySecureShell installation easily with ``sftp-verif`` tool. It will check if every needed binaries are present with correct rights and will check the configuration. You simply have to run it::

    > sftp-verif 

    ################################################################################
                MySecureShell Verification Tool
    ################################################################################

    ### Verifing file existance ###

    /usr/bin/mysecureshell                                                   [ OK ]
    /bin/sftp-who                                                            [ OK ]
    /bin/sftp-kill                                                           [ OK ]
    /bin/sftp-state                                                          [ OK ]
    /bin/sftp-admin                                                          [ OK ]
    /bin/sftp-verif                                                          [ OK ]
    /bin/sftp-user                                                           [ OK ]


    ### Verifing /etc/shells ###

    /etc/shells contains /usr/bin/mysecureshell                              [ OK ]


    ### Verifing rights ###

    Verifing file rights of /etc/ssh/sftp_config                             [ OK ]
    Verifing file rights of /bin/sftp-who                                    [ OK ]
    Verifing file rights of /bin/sftp-verif                                  [ OK ]
    Verifing file rights of /bin/sftp-user                                   [ OK ]
    Verifing file rights of /bin/sftp-kill                                   [ OK ]
    Verifing file rights of /bin/sftp-state                                  [ OK ]
    Verifing file rights of /bin/sftp-admin                                  [ OK ]
    Verifing file rights of /usr/bin/mysecureshell                           [ OK ]


    ### Verifing rotation logs ###

    Rotation logs have been found                                            [ OK ]


    ### Verifing server status ###

    Verifing server status (ONLINE)                                          [ OK ]


    ### Verifing server dependencies ###

    Show only error(s) :


    ### Verifing server configuration ###

    Show only error(s) :
    Trying user: root
    Checking user : mssuser


    ### All tests dones ###

Manager users for MySecureShell
-------------------------------

You can easily add a new user to MySecureShell by using the ``sftp-user`` command::

    > sftp-user create mssuser /home/mssuser
    Enter password:

This will create the user and password, then add MySecureShell shell to this user.

.. note:: ``sftp-user`` uses the common binary tool to create users (useradd on Linux and niutil on Mac OS).

You can also delete a user or getting the list of avaialable users::

    > sftp-user list
    mssuser

Here is the list of available options::

    > sftp-user --help
    Usage 1: sftp-user create [user] [hide user]
    Usage 2: sftp-user delete [user]
    Usage 3: sftp-user list
    Usage 4: sftp-user hide [user] [hide] (Mac OS X Only)


        Options     Choices     Descriptions
        user        Username    User Name
        hide user   1/0     hide user from login panel and user managements (Max OS X Only)
        delete home 1/0     Remove user Home directory
        hide        1/0     Hide/Unhide (Mac OS X Only)

