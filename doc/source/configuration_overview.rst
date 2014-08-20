Overview
========

.. contents:: Topics

.. highlight:: apache

To configure MySecureShell, you need to edit the */etc/ssh/sftp_config* file. By default MySecureSHell comes with a commented example configuration. This may not match all your needs and you will need to modify it to get it working as expected.

The configuration is made with 2 importants kinds of tags:

* Parent tags (level 1): defining a global context
* Child tags (level 2): defining a specific option for the parent tag

Configuration files should looks like this::

    <ParentTag [parameter]>
        ChildTag    value
    </Parent>

Parent tags
-----------

===========               =========== =======
Tags                      Values      Summary
===========               =========== =======
Default                   None        Default values will be applyed for all other parents tags
FileSpec                  filter name Allow to create filter on files
Group                     group name  Allow a group
IpRange                   IP/CIDR     Allow a set of range IP address
User                      user name   Allow a user
VirtualHost               FQDN        Allow a VirtualHost
===========               =========== =======

Child tags
----------

====================== ================= ===============
Tags                   Values            Summary
====================== ================= ===============
ByPassGlobalDownload   false/true        Bypassing GlobalDownload value
ByPassGlobalUpload     false/true        Bypassing GlobalUpload value
CanChangeRights        false/true        Allow to change rights on files and directories
CanChangeTime          false/true        Allow to change access and creation time on files and directories
CanRemoveDir           false/true        Allow to remove directories
CanRemoveFile          false/true        Allow to remove files
Charset                <charset>         Enable special charset compatibility
ConnectionMaxLife      <integer>          Limit maximum connection time in seconds
CreateHome             false/true        Create home user's directory if it doesn't exist
DefaultRights          <umask>           Set default rights on new files and folders created (`umask format <http://en.wikipedia.org/wiki/Umask>`_
DirFakeGroup           false/true        Substitute shown group owner name of files and directories, by the name of the connected user
DirFakeMode            xxxx              Substitute shown rights of files and directories, by those ones
DirFakeUser            false/true        Substitute shown user owner name of files and directories, by the name of the connected user
DisableAccount         false/true        Quickly disable an account
DisableMakeDir         false/true        Disable new directories creation
DisableOverwrite       false/true        Disable file's overwriting
DisableReadDir         false/true        Disable reading directories
DisableReadFile        false/true        Disable reading files
DisableRemoveDir       false/true        Disable removing directories
DisableRemoveFile      false/true        Disable removing files
DisableRename          false/true        Disable renaming files and directories
DisableSetAttribute    false/true        Disable changing attributes on files and directories
DisableSymLink         false/true        Disable creating symbolic link
DisableWriteFile       false/true        Disable writing files
Download               <integer><units>   Download bandwidth by connected user
ExpireDate             <date>            Expiration date
ForceGroup             <groupname>       
ForceRights            Umask             
ForceUser              user              
GlobalDownload         <integer><units>   Maximum allowed bandwidth in download for the server
GlobalUpload           <integer><units>   Maximum allowed bandwidth in upload for the server
HideNoAccess           false/true        Hide not allowed permissions files and directories
Home                   <path>            Change home of users.
IdleTimeOut            <integer>          Inactivity timeout before deconnection in seconds
IgnoreHidden           false/true        Show hidden files and directories (starting with a dot)
Include                <path>            Include another configuration file
IsAdmin                false/true        Allow to be administrator
IsSimpleAdmin          false/true        Allow to be administrator (less righs than IsAdmin tag)
LimitConnection        <integer>          Limit the number of simultaneous connections
LimitConnectionByIp    <integer>          Limit the number of simultaneous connections by IP
LimitConnectionByUser  <integer>          Limit the number of simultaneous connections by user
LogFile                <path>            Allows to change the file of log
LogSyslog              false/true        Write log to syslog
MaxOpenFilesForUser    <integer>          Limit maximum opening files simultaneously
MaxReadFilesForUser    <integer>          Limit maximum reading files simultaneously
MaxWriteFilesForUser   <integer>          Limit maximum writing files simultaneously
MaximumRights          <umask>           Set a maximum rights on new files and folders created (`umask format <http://en.wikipedia.org/wiki/Umask>`_
MinimumRights          <umask>           Force minimum rights for new files and new directories (`umask format <http://en.wikipedia.org/wiki/Umask>`_
ResolveIP              false/true        Resolve IP address from DNS
SftpProtocol           <integer>          Force the SFTP protocol version
Shell                  <path>            Specify a Shell path to allow users to have a real shell
ShowLinksAsLinks       false/true        See symbolic links as true files or folders
StayAtHome             false/true        Restrict user to stay in its home directory and subdirectories
Upload                 x(units)          Upload bandwidth by connected user
VirtualChroot          false/true        Chroot users in their Home directory
====================== ================= ===============

Deprecated tags
---------------
*Those tags are deprecated and don't work anymore!*

====================== ================= ===============
Tags                   Values            Summary
====================== ================= ===============
GMTTime                <+/-><integer>     Adjusts the clock of the log
HideFiles              <regex>           Hide files / directories that you want using a regular expression (regex)
PathAllowFilter        <regex>           Restricted access to files / directories using regular expression (regex)
PathDenyFilter         <path>            Do not authorize files/directories which match with the regular expression
====================== ================= ===============
