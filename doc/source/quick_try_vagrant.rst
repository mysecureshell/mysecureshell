Vagrant
=======

.. image:: images/logo_vagrant.png
    :align: center
    :width: 400px

.. contents:: Topics

.. highlight:: bash

Vagrant is a solution to create and configure lightweight, reproducible, and portable development environments. It uses a virtualization solution like VirtualBox. The advantage of this solution is it works on many Operating Systems (Linux, Mac OS and Windows).

Install prequesites
-------------------

How does it works? First of all, you need install those elements:

#. `VirtualBox <http://www.virtualbox.org/>`_: the virtualization tool
#. `Vagrant <http://www.vagrantup.com/>`_: easily deploy a MySecureShell testing environment

Add Vagrantfile
---------------

Once done, on your machine create a folder and add get the Vagrantfile::

    $ mkdir mysecureshell
    $ cd mysecureshell
    $ wget --no-check-certificate https://github.com/deimosfr/mysecureshell/master/deployment-tools/vagrant/Vagrantfile

Here is the content of the Vagrantfile:

.. literalinclude:: ../../deployment-tools/vagrant/Vagrantfile
   :language: ruby

Boot the Virtual Machine
------------------------

As you can see, several network interfaces are available for your tests and an installation part will install MySecureShell with a dedicated user. You can now boot the Virtual Machine::

    $ vagrant up
    ==> default: Forcing shutdown of VM...
    ==> default: Destroying VM and associated drives...
    ==> default: Running cleanup tasks for 'shell' provisioner...
    Bringing machine 'default' up with 'virtualbox' provider...
    ==> default: Importing base box 'deimosfr/debian-wheezy'...
    ==> default: Matching MAC address for NAT networking...
    ...
    ==> default: Server is now online.
    ==> default: Starting SFTP Server: 
    ==> default: MySecureShell is now online

Connect and test
----------------

MySecureShell is now ready to serve! From your host machine, you can connect with user *mssuser* and *mssuser* for the password::

    $ sftp mssuser@192.168.33.10
    mssuser@192.168.33.10's password: 
    Connected to 192.168.33.10.
    sftp> ls
    sftp> pwd
    Remote working directory: /

In parallel, connect from your host machine to the Virtual Machine and see the current connected user with `sftp-who` command::

    $ vagrant ssh
    $ sudo sftp-who
    --- 1 / 10 clients ---
    Global used bandwith : 0 bytes/s / 0 bytes/s
    PID: 3389   Name: mssuser   IP: 
        Home: /home/mssuser
        Status: idle    Path: /
        File: 
        Connected: 2014/08/19 15:38:27 [since 10s]
        Speed: Download: 0 bytes/s [5.00 kbytes/s]  Upload: 0 bytes/s [unlimited]
        Total: Download: 1398 bytes   Upload: 141 bytes

You can see the connected user :-). You can try to upload files to see the result. Of course you can use graphical clients. And if you want to play with the server configuration, look at ``/etc/ssh/sftp_config``.
