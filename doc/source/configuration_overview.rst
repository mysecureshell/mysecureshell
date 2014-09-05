Overview
========

.. contents:: Topics

.. highlight:: apache

To configure MySecureShell, you need to edit the */etc/ssh/sftp_config* file. By default MySecureShell comes with a commented example configuration. This may not match all your needs and you will need to modify it to get it working as expected.

The configuration is made with 2 importants kinds of tags:

* Parent tags (level 1): defining a global context
* Child tags (level 2): defining a specific option for the parent tag

Configuration files should looks like this::

    <ParentTag [parameter]>
        ChildTag    value
    </Parent>

Parent tags
-----------

=============              =========== =======
Tags                       Values      Summary
=============              =========== =======
Default_                   None        Default values will be applyed for all other parents tags
FileSpec_                  filter name Allow to create filter on files
Group_                     group name  Allow a group
IpRange_                   IP/CIDR     Allow a set of range IP address
User_                      user name   Allow a user
VirtualHost_               FQDN        Allow a VirtualHost
=============              =========== =======

.. _Default: tags/parents/default.html
.. _FileSpec: tags/parents/filespec.html
.. _Group: tags/parents/group.html
.. _IpRange: tags/parents/iprange.html
.. _User: tags/parents/user.html
.. _VirtualHost: tags/parents/virtualhost.html

Child tags
----------

====================== ================= ===============
Tags                   Values            Summary
====================== ================= ===============
ByPassGlobalDownload_  false/true        Bypassing GlobalDownload value
ByPassGlobalUpload_    false/true        Bypassing GlobalUpload value
CanChangeRights_       false/true        Allow to change rights on files and directories
CanChangeTime_         false/true        Allow to change access and creation time on files and directories
Charset_               <charset>         Enable special charset compatibility
ConnectionMaxLife_     <unix rights>     Limit maximum connection time in seconds
CreateHome_            false/true        Create home user's directory if it doesn't exist
DefaultRights_         <unix rights>     Set default rights on new files and folders created
DirFakeGroup_          false/true        Substitute shown group owner name of files and directories, by the name of the connected user
DirFakeMode_           xxxx              Substitute shown rights of files and directories, by those ones
DirFakeUser_           false/true        Substitute shown user owner name of files and directories, by the name of the connected user
DisableAccount_        false/true        Quickly disable an account
DisableMakeDir_        false/true        Disable new directories creation
DisableOverwrite_      false/true        Disable file's overwriting
DisableReadDir_        false/true        Disable reading directories
DisableReadFile_       false/true        Disable reading files
DisableRemoveDir_      false/true        Disable removing directories
DisableRemoveFile_     false/true        Disable removing files
DisableRename_         false/true        Disable renaming files and directories
DisableSetAttribute_   false/true        Disable changing attributes on files and directories
DisableSymLink_        false/true        Disable creating symbolic link
DisableWriteFile_      false/true        Disable writing files
Download_              <integer><units>  Download bandwidth by connected user
ExpireDate_            <date>            Expiration date
ForceGroup_            <groupname>       Force group assignment for connected user
ForceRights_           <unix rights>     Force rights assignment for connected user
ForceUser_             user              Force user assignment for connected user
GlobalDownload_        <integer><units>  Maximum allowed bandwidth in download for the server
GlobalUpload_          <integer><units>  Maximum allowed bandwidth in upload for the server
HideNoAccess_          false/true        Hide not allowed permissions files and directories
Home_                  <path>            Change home of users.
IdleTimeOut_           <integer>         Inactivity timeout before deconnection in seconds
IgnoreHidden_          false/true        Show hidden files and directories (starting with a dot)
Include_               <path>            Include another configuration file
IsAdmin_               false/true        Allow to be administrator
IsSimpleAdmin_         false/true        Allow to be administrator (less righs than IsAdmin tag)
LimitConnection_       <integer>         Limit the number of simultaneous connections
LimitConnectionByIp_   <integer>         Limit the number of simultaneous connections by IP
LimitConnectionByUser_ <integer>         Limit the number of simultaneous connections by user
LogFile_               <path>            Allows to change the file of log
LogSyslog_             false/true        Write log to syslog
MaxOpenFilesForUser_   <integer>         Limit maximum opening files simultaneously
MaxReadFilesForUser_   <integer>         Limit maximum reading files simultaneously
MaxWriteFilesForUser_  <integer>         Limit maximum writing files simultaneously
MaximumRights_         <unix rights>     Set a maximum rights on new files and folders created (`unix rights format <http://en.wikipedia.org/wiki/unix rights>`_)
MinimumRights_         <unix rights>     Force minimum rights for new files and new directories (`unix rights format <http://en.wikipedia.org/wiki/unix rights>`_)
ResolveIP_             false/true        Resolve IP address from DNS
SftpProtocol_          <integer>         Force the SFTP protocol version
Shell_                 <path>            Specify a Shell path to allow users to have a real shell
ShowLinksAsLinks_      false/true        See symbolic links as true files or folders
StayAtHome_            false/true        Restrict user to stay in its home directory and subdirectories
Upload_                x(units)          Upload bandwidth by connected user
VirtualChroot_         false/true        Chroot users in their Home directory
====================== ================= ===============

.. _ByPassGlobalDownload: tags/childs/bypassglobaldownload.html
.. _ByPassGlobalUpload: tags/childs/bypassglobalupload.html
.. _CanChangeRights: tags/childs/canchangerights.html
.. _CanChangeTime: tags/childs/canchangetime.html
.. _Charset: tags/childs/charset.html
.. _ConnectionMaxLife: tags/childs/connectionmaxlife.html
.. _CreateHome: tags/childs/create_home.html
.. _DefaultRights: tags/childs/defaultrights.html
.. _DirFakeGroup: tags/childs/dirfakegroup.html
.. _DirFakeMode: tags/childs/dirfakemode.html
.. _DirFakeUser: tags/childs/dirfakeuser.html
.. _DisableAccount: tags/childs/disableaccount.html
.. _DisableMakeDir: tags/childs/disablemakedir.html
.. _DisableOverwrite: tags/childs/disableoverwrite.html
.. _DisableReadDir: tags/childs/disablereaddir.html
.. _DisableReadFile: tags/childs/disablereadfile.html
.. _DisableRemoveDir: tags/childs/disableremovedir.html
.. _DisableRemoveFile: tags/childs/disableremovefile.html
.. _DisableRename: tags/childs/disablerename/disablerename.html
.. _DisableSetAttribute: tags/childs/disablesetattribute.html
.. _DisableSymLink: tags/childs/disablesymlink/disablesymlink.html
.. _DisableWriteFile: tags/childs/disablewritefile.html
.. _Download: tags/childs/download.html
.. _ExpireDate: tags/childs/expiredate.html
.. _ForceGroup: tags/childs/forcegroup.html
.. _ForceRights: tags/childs/forcerights.html
.. _ForceUser: tags/childs/forceuser.html
.. _GlobalDownload: tags/childs/globaldownload.html
.. _GlobalUpload: tags/childs/globalupload.html
.. _HideNoAccess: tags/childs/hidenoaccess.html
.. _Home: tags/childs/home.html
.. _IdleTimeOut: tags/childs/idletimeout.html
.. _IgnoreHidden: tags/childs/ignorehidden.html
.. _Include: tags/childs/include.html
.. _IsAdmin: tags/childs/isadmin.html
.. _IsSimpleAdmin: tags/childs/issimpleadmin.html
.. _LimitConnection: tags/childs/limitconnection.html
.. _LimitConnectionByIp: tags/childs/limitconnectionbyip.html
.. _LimitConnectionByUser: tags/childs/limitconnectionbyuser.html
.. _LogFile: tags/childs/logfile.html
.. _LogSyslog: tags/childs/logsyslog.html
.. _MaxOpenFilesForUser: tags/childs/maxopenfilesforuser.html
.. _MaxReadFilesForUser: tags/childs/maxreadfilesforuser.html
.. _MaxWriteFilesForUser: tags/childs/maxwritefilesforuser.html
.. _MaximumRights: tags/childs/maximumrights.html
.. _MinimumRights: tags/childs/minimumrights.html
.. _ResolveIP: tags/childs/resolveip.html
.. _SftpProtocol: tags/childs/sftpprotocol.html
.. _Shell: tags/childs/shell.html
.. _ShowLinksAsLinks: tags/childs/showlinksaslinks.html
.. _StayAtHome: tags/childs/stayathome.html
.. _Upload: tags/childs/upload.html
.. _VirtualChroot: tags/childs/virtualchroot.html

Deprecated tags
---------------
*Those tags are deprecated and don't work anymore!*

====================== ================= ===============
Tags                   Values            Summary
====================== ================= ===============
CanRemoveDir_          false/true        Allow to remove directories
CanRemoveFile_         false/true        Allow to remove files
GMTTime_               <+/-><integer>    Adjusts the clock of the log
HideFiles_             <regex>           Hide files / directories that you want using a regular expression (regex)
PathAllowFilter_       <regex>           Restricted access to files / directories using regular expression (regex)
PathDenyFilter_        <path>            Do not authorize files/directories which match with the regular expression
====================== ================= ===============

.. _CanRemoveDir: tags/deprecated/canremovedir.html
.. _CanRemoveFile: tags/deprecated/canremovefile.html
.. _GMTTime: tags/deprecated/gmttime.html
.. _HideFiles: tags/deprecated/hidefiles.html
.. _PathAllowFilter: tags/deprecated/pathallowfilter.html
.. _PathDenyFilter: tags/deprecated/pathdenyfilter.html

