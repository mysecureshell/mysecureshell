FileSpec
========

.. highlight:: apache

Synopsis
--------
FileSpec is made to create filters on files and directories. You need to create a parent rule with a dedicated and unique name and apply it as a child tag in any other parent tag.

Options
-------

========== ========= ======== ============= =======
Name       Default   Values   Since version Context
========== ========= ======== ============= =======
FileSpec   null      name     1.30          Filters for any parent tags
========== ========= ======== ============= =======

Examples
--------
In this example, we created a FileSpec called AllowedExtensions which deny accessing to files with *.exe* or *.sh* extensions::

    <FileSpec AllowedExtensions>
        # Only check against filenames/folder names only
        UseFullPath false

        # we can use multiple deny/allow directives for clarity
        Order DenyAllow
        Deny ".*.exe$"
        Deny ".*.sh$"
        Allow all
    </FileSpec>

As the second example, we want to force users to only have access to data and home directories. For that we're using regex::

    <FileSpec OnlyDataAndHome>
        # Here, we check against the full absolute path
        # instead of just the filename or foldername
        UseFullPath true

        # We define the order in which paths are evaluated
        Order AllowDeny

        # Allow any full path that starts with /data or /home
        Allow "^/(data|home)"
        Deny all
    </FileSpec>

To finish, we want to apply those filters in the default configuration::

    <Default>
        ApplyFileSpec OnlyDataAndHome,AllowedExtensions
    </Default>

As you seen, you can add with a separated comma multiple *FileSpec* filters at once.
