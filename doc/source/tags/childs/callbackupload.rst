CallbackDownload
================

.. highlight:: apache

Synopsis
--------
This tag allows calling custom shell command on the server when user succesfully uploads a file. Working directory is not changed. Usual environment variables can be used from the command, such as USER, HOME, SSH_CONNECTION; path of the file is kept in LAST_FILE_PATH. Shell command result is written to log.

Options
-------

================ ========= ========= ============= =======
Name             Default   Values    Since version Context
================ ========= ========= ============= =======
CallbackUpload   null      shell cmd 2.1           Default,Group,IpRange,User,VirtualHost    
================ ========= ========= ============= =======

Examples
--------
Here, we use "env" system call to output environment variables in the log::
    <Default>
        CallbackUpload "env"
    </Default>

And this would call custom script::
    <Default>
        CallbackUpload "/bin/my-script.sh $LAST_FILE_PATH"
    </Default>
