#! /usr/bin/env python

# MySecureShell package creation tool
# This tool helps on creating packages for multiple distributions

# Requirements:
# - Python Dialog library (apt-get install python-dialog)

# Todo
# - Build images
# - Use API instead of system command to call docker
# - Finish to add all listed features in dialog

import sys
import os
import dialog
import time

# Global vars
docker_folder = '../deployment-tools/docker'


def handle_exit_code(d, code):
    """Sample function showing how to interpret the dialog exit codes.

    This function is not used after every call to dialog in this demo
    for two reasons:

       1. For some boxes, unfortunately, dialog returns the code for
          ERROR when the user presses ESC (instead of the one chosen
          for ESC). As these boxes only have an OK button, and an
          exception is raised and correctly handled here in case of
          real dialog errors, there is no point in testing the dialog
          exit status (it can't be CANCEL as there is no CANCEL
          button; it can't be ESC as unfortunately, the dialog makes
          it appear as an error; it can't be ERROR as this is handled
          in dialog.py to raise an exception; therefore, it *is* OK).

       2. To not clutter simple code with things that are
          demonstrated elsewhere.

    """
    # d is supposed to be a Dialog instance
    if code in (d.DIALOG_CANCEL, d.DIALOG_ESC):
        main_menu(d)
        return 0
    else:
        return 1


def update_docker_containers(d):
    """
    Pull or upgrade docker containers

    :returns: exit code
    """

    make_choices = [("All", "", 0)]
    distro_list, versions_list = get_distro_list()
    for distro in distro_list:
        make_choices.append([distro, '', 0])
    mk_len = len(make_choices)

    tag = []
    while len(tag) == 0:
        while 1:
            (code, tag) = d.checklist(text=' '.join(['Which containers would',
                                                     'you like to pull or',
                                                     'upgrade ? It will',
                                                     'install all',
                                                     'dependencies as well !']),
                                      height=mk_len+8, width=60,
                                      list_height=mk_len,
                                      choices=make_choices,
                                      title="Update Docker containers")
            if handle_exit_code(d, code):
                break

    # Update desired tag
    if tag[0] == 'All':
        tag = distro_list
    for cur_tag in tag:
        docker_pull(d, cur_tag.lower())

    main_menu(d)


def build_docker_containers(d):
    """
    Build Docker containers from Dockerfiles

    :returns: exit code
    """
    make_choices = [("All", "", 0)]
    distro_list, versions_list = get_distro_list()
    for distro in versions_list:
        make_choices.append([distro, '', 0])
    mk_len = len(make_choices)

    tag = []
    while len(tag) == 0:
        while 1:
            (code, tag) = d.checklist(text=' '.join(['Which containers would',
                                                     'you like to build? It',
                                                     'will install all',
                                                     'MySecureShell',
                                                     'dependencies']),
                                      height=mk_len+8, width=60,
                                      list_height=mk_len,
                                      choices=make_choices,
                                      title="Build Docker containers")
            if handle_exit_code(d, code):
                break

    # Update desired tag
    if tag[0] == 'All':
        tag = distro_list
    for cur_tag in tag:
        docker_build(d, cur_tag.lower())

    main_menu(d)


def docker_build(d, cur_tag):
    """
    Build with prefixed 'mss_', Docker containers from Dockerfiles

    :d: dialog backtitle
    :cur_tag: current tag containing distro and version (space separated)

    """
    try:
        os.system('docker build -t mss_' + cur_tag.replace(' ', '_') + ' ' +
                  docker_folder + '/' + cur_tag.replace(' ', '/'))
        d.infobox('Successful build for ' + cur_tag, height=4, width=50)
        time.sleep(1.5)
    except:
        print "Can't use docker"
        sys.exit(1)


def docker_pull(d, cur_tag):
    """@todo: Docstring for docker_pull.

    :tag: list of docker to update
    :d: dialog backtitle
    :returns: @todo

    """
    try:
        print 'Getting ' + cur_tag
        os.system('docker pull ' + cur_tag)
    except:
        print "Can't use docker"
        sys.exit(1)


def show_vars(d):
    """
    Show vars from vars file

    :d: dialog backtitle

    """
    d.textbox("vars", width=76)
    main_menu(d)


def get_distro_list():
    """
    Get the distributions list from filesystems folders

    :d: dialog backtitle
    :returns: list

    """
    distro_list = []
    versions_list = []

    os.chdir('../deployment-tools/docker')
    for dirname, dirnames, filenames in os.walk('.'):
        releases = dirname.split('/')
        cur_len_release = len(releases)
        if cur_len_release == 3:
            versions_list.append(' '.join([releases[1], releases[2]]))
        elif cur_len_release == 2:
            distro_list.append(releases[1])
    os.chdir('../../packaging')
    return(sorted(distro_list), sorted(versions_list))


def make_pkg(d):
    """@todo: Docstring for make_pkg.

    :d: dialog backtitle

    """

    # Create the list with docker folders
    archs = ('x64', 'x86')
    make_choices = [("All", "", 0)]
    distro_list, versions_list = get_distro_list()
    for distro in versions_list:
        for current_archs in archs:
            make_choices.append((' '.join([distro, current_archs]), "", 0))
    mk_len = len(make_choices)

    # Show menu to select wished packages
    tag = []
    while len(tag) == 0:
        while 1:
            (code, tag) = d.checklist(text="Select distrib to build package on",
                                      height=mk_len+8, width=60,
                                      list_height=mk_len,
                                      choices=make_choices,
                                      title="Make packages")
            if handle_exit_code(d, code):
                break

    # Update desired tag
    if tag[0] == 'All':
        tag = make_choices
        del tag[0]
    for cur_tag in tag:
        print 'Running container ' + cur_tag[0]

    sys.exit(1)
    main_menu(d)


def run_docker(env):
    """
    Run a docker container to prepare compilation. Get docker from
    deployment-tools folder if does not exist and launch MySecureShell
    compilation and package creation.

    :env: docker environment to run
    :returns: @todo

    """
    try:
        container = ' '.split(env)
        os.system('docker run -t -i' + container)
    except:
        print "Can't use docker"
        sys.exit(1)


def show_mss_containers(d):
    """
    Show MySecureShell Docker containers available on the current machine

    :d: dialog backtitle
    :returns: @todo

    """
    current_mss_images = os.system("docker images | awk '/mss_/{print $1}'")
    print current_mss_images
    d.msgbox(current_mss_images, height=10, width=10)
    sys.exit(0)


def main_menu(d):
    """
    Main menu

    :d: dialog backtitle

    :returns: exit code
    """
    while 1:
        (code, tag) = d.menu(
            "Make your choice",
            height=15, width=60, menu_height=9,
            choices=[("1 Update Docker containers", "Get/Update containers"),
                     ("2 Build Docker containers", "Build from Dockerfiles"),
                     ("3 Create packages", "For Debian/Ubuntu/Centos..."),
                     ("4 Make documentations", "In PDF/HTML..."),
                     ("", ""),
                     ("Show installed images", "Only MySecureShell ones"),
                     ("Show vars", "Show current vars"),
                     ("Exit", "")])
        if handle_exit_code(d, code):
            break

    if tag == 'Exit':
        sys.exit(0)
    elif tag == '1 Update Docker containers':
        update_docker_containers(d)
    elif tag == '2 Build Docker containers':
        build_docker_containers(d)
    elif tag == 'Show installed images':
        show_mss_containers(d)
    elif tag == 'Show vars':
        show_vars(d)
    elif tag == 'Create packages':
        make_pkg(d)

    sys.exit(0)


def main():
    """
    Main function

    :returns: exit code

    """
    try:
        d = dialog.Dialog(dialog="dialog")
        d.add_persistent_args(["--backtitle", 'MySecureShell Packaging/' +
                               'Documentation creator'])
        main_menu(d)
    except dialog.error, exc_instance:
        sys.stderr.write("Error:\n\n%s\n" % exc_instance.complete_message())
        sys.exit(1)

    sys.exit(0)


if __name__ == "__main__":
    main()
