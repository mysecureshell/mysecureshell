Contribute
==========

.. contents:: Topics

.. highlight:: bash

You may be interrested in contribute to MySecureShell project. We're going to see here how you can do this :-)

Which kind of contribution are accepted?
----------------------------------------

Any kind of contribution is welcome in fact :-). 

* If it's a bug or feature related, do not hesitate to use GitHub issues to inform us.
* You created a patch for MySecureShell? Please use GitHub Pull Request to provide it.
* You made a plugin for a third party application? Please inform us! And if you want we host it, do not hesitate to inform us.
* The documentation is not good enough and you want to enhance it? Please make the changes from the sources by using a Pull Request
* The documentation is not translated into your language? You can have a look to Transifex website and we will integrate it.
* We did not make the package for your favorite distribution? Create your own and inform us how to install it, we will add it to the documentation!

As you can see, we're open to any kind of contribution and hope you'll enjoy contributing.

.. _code_contribute:

What is the easiest solution to contribute code or doc?
-------------------------------------------------------

We are providing Docker and Vagrant files that will install everything you need to have (packages, sources, dependancies, documentation sources and so on...) to help you to contribute.

Docker
^^^^^^

With Docker, here is the content file for development:

.. literalinclude:: ../../deployment-tools/docker/debian/unstable/Dockerfile
   :language: bash

To install this container, simply build docker with the Dockerfile::

    $ docker build -t mysecureshelldev \
    https://raw.githubusercontent.com/mysecureshell/mysecureshell/master/deployment-tools/docker/dev/Dockerfile

Now you can run one::

    $ docker run -d -p 22222:22 mysecureshelldev

If you need more explaination on how to use it, please look at the :doc:`Quick Try <quick_try_docker>` section.

Vagrant
^^^^^^^

With Vagrant and the Vagrantfile:

.. literalinclude:: ../../deployment-tools/vagrant/Vagrantfile
   :language: ruby

you just need to uncomment this line to install all development dependencies:

.. code-block:: ruby
    
    #config.vm.provision "shell", inline: $install_mss_dev

Then launch it::

    $ vagrant up

If you need more explaination on how to use it, please look at the :doc:`Quick Try <quick_try_vagrant>` section.

How can I contribute to the documentation?
------------------------------------------

The simplest solution is to look at the :ref:`previous section <code_contribute>` to create a development environment with all the prequesites to build the documentation.

All the documentation is based in the *doc* folder. The documentation has been made with `Sphinx Doc <http://sphinx-doc.org/>`_, which is easy to use.

When you've got the development environment and made modifications in the documentation, you can try to generate a new one with this command inside the *doc* folder::

    $ make html
    sphinx-build -b html -d build/doctrees   source build/html
    Running Sphinx v1.2.2
    loading pickled environment... done
    loading intersphinx inventory from http://docs.python.org/objects.inv...
    building [html]: targets for 1 source files that are out of date
    updating environment: 0 added, 1 changed, 0 removed
    reading sources... [100%] contribute                                                                                
    looking for now-outdated files... 77 found
    pickling environment... done
    checking consistency... done
    preparing documents... done
    writing output... [100%] usages_user                                                                                
    writing additional files... genindex search
    copying images... [100%] images/logo_ubuntu.png                                                                     
    copying static files... done
    copying extra files... done
    dumping search index... done
    dumping object inventory... done
    build succeeded, 1 warning.

    Build finished. The HTML pages are in build/html.


This will generate an html documentation available in the *doc/build/html* folder. Once done, you can create a pull request for merging your modifications.

Share your MySecureShell usage and knowledge
--------------------------------------------

If you like MySecureShell, do not hesitate to make blog posts, social sharing (Twitter, Google+, Facebook...) or participate with us to help users on GitHub issues.

.. note:: MySecureShell is available on Twitter with `@MySecureShell <https://twitter.com/mysecureshell>`_ username.

You can also help us on testing futures releases by participating to beta tests.
