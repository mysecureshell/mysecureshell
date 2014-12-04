#! /usr/bin/env python

# MySecureShell package creation tool
# This tool helps on creating packages for multiple distributions

# Requirements:
# - Python Dialog library (apt-get install python-dialog)
# - Docker >= 1.3

# Todo
# - Review the code to make it cleaner
# - use popen instead of system
# - catch all returns
# - log everything in a log file
# - remove all
# - Use API instead of system command to call docker -> no
#   http://blog.bordage.pro/avoid-docker-py/

import sys
import os
import dialog
import time
import subprocess
import re
import shutil

# Global vars
docker_folder = '../deployment-tools/docker'
packaging_path = os.getcwd()
dest_folder = '/root/mysecureshell'
pkg_folder = dest_folder + '/packaging/'


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


def build_docker_containers(d):
    """
    Build Docker containers from Dockerfiles

    :returns: exit code
    """
    make_choices = [("All", "", 0)]
    distro_list, versions_list = get_distro_list()
    for version in versions_list:
        make_choices.append([version, '', 0])
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


def docker_build(d, cur_tag):
    """
    Build with prefixed 'mss_', Docker containers from Dockerfiles

    :d: dialog backtitle
    :cur_tag: current tag containing distro and version (space separated)

    """
    try:
        os.system('docker build --no-cache -t mss_' +
                  cur_tag.replace(' ', '_') + ' ' +
                  docker_folder + '/' + cur_tag.replace(' ', '/'))
        d.infobox('Successful built ' + cur_tag, height=4, width=50)
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
    os.chdir(packaging_path)
    return(sorted(distro_list), sorted(versions_list))


def copy_gpg(d, docker_name):
    """
    Copy GPG keys
    If a current .gnupg is available inside the current directory, it will pass.
    If nothing's found, it will try to copy .gnupg folder inside your home
    directory to the current directory.
    Then it will copy this to the container ~root directory

    :d: dialog backtitle
    :docker_name: name of the container

    """
    errors = []
    gnupg_path = os.path.expanduser('~') + '/.gnupg'
    current_path = os.getcwd()

    # Check if the directory already exist or not
    if os.path.isdir(current_path + '/.gnupg'):
        print 'A .gnupg folder already exist in the current dir'
    # Copy GPG folder from home directory if exist
    elif os.path.isdir(gnupg_path):
        try:
            dest_gnupg = current_path + '/.gnupg'
            for src_dir, dirs, files in os.walk(gnupg_path):
                if not os.path.exists(dest_gnupg):
                    os.mkdir(dest_gnupg)
                for file_ in files:
                    src_file = os.path.join(gnupg_path, file_)
                    shutil.copy2(src_file, dest_gnupg)
        except (IOError, os.error) as why:
            errors.append((gnupg_path, '.', str(why)))
    else:
        print gnupg_path + ' folder does not exist'
        return

    docker_exec(d, docker_name, 'cp -Rf /mnt/packaging/.gnupg ' +
                '/root/', 'Copying GPG keys ' + str(docker_name))


def create_packages(d):
    """
    Create packages

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

    # Choose the desired tag/branch to build
    git_version = select_version_to_build(d)

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
    # Get timestamp to create uniq containers
    timest = int(time.time())
    for cur_tag in tag:
        # Set os vars
        (distro, distro_version, arch) = cur_tag.split(' ')
        # Create a container
        docker_name = run_docker(d, distro, distro_version, timest)
        # Copy sources to container
        docker_exec(d, docker_name, 'cp -Rf /mnt /root/mysecureshell',
                    'Copying source folder to ' + str(docker_name))
        # Copy gpg key
        copy_gpg(d, docker_name)
        # Switch to the correct branch/tag
        docker_exec(d, docker_name,
                    'git --git-dir=' + dest_folder + '/.git --work-tree=' +
                    dest_folder + ' checkout ' + git_version,
                    'Switching to "' + str(git_version) + '" branch/tag')
        # Copy latest version of the packaging folder
        docker_exec(d, docker_name, 'cp -Rf /mnt/packaging ' + dest_folder,
                    'Replacing packaging folder with the one on your host')
        # Launch builder script
        de_return = docker_exec(d, docker_name,
                                pkg_folder + distro + '/' + distro_version +
                                '/build.sh ' + git_version + ' ' + pkg_folder,
                                'Building package')
        # Copy packages to host
        dest_dir = 'package/' + distro + '_' + distro_version
        if not os.path.exists(dest_dir):
            os.makedirs('package/' + distro + '_' + distro_version)
        os.system('docker cp ' + docker_name + ':' + pkg_folder + '/package ' +
                  'package/' + distro + '_' + distro_version)


def docker_exec(d, docker_name, cmd, comment):
    """
    Docker execution command with 'docker exec' command

    :d: dialog backtitle
    :docker_name: container name
    :cmd: command to execute in the container
    :comment: display comment
    :returns: @todo

    """
    docker_exec = 'docker exec -t -i ' + docker_name + ' ' + cmd

    try:
        d.infobox(comment, height=4, width=78)
        print docker_exec
        os.system(docker_exec)
        time.sleep(1.5)
        return 0
    except:
        print "Can't run docker: " + docker_exec
        sys.exit(1)


def run_docker(d, distro, version, timest):
    """
    Run a docker container to prepare compilation. Get docker from
    deployment-tools folder if does not exist and launch MySecureShell
    compilation and package creation.

    :d: dialog backtitle
    :distro: current distro
    :version: current version of the distro
    :timest: timestamp
    :returns: @todo

    """
    os.chdir('..')
    current_path = os.getcwd()

    docker_name = 'mss_' + str(distro) + '_' + str(version) + '_' + str(timest)
    docker_run = 'docker run -d -t -v ' + current_path +\
                 ':/mnt --name=' + docker_name +\
                 ' mss_' + str(distro) + '_' + str(version) +\
                 ' 1>/dev/null'

    try:
        d.infobox('Building container ' + str(distro) + ' ' + str(version),
                  height=4, width=50)
        os.system(docker_run)
        time.sleep(1.5)
        os.chdir(packaging_path)
    except:
        print "Can't run docker: " + docker_run
        sys.exit(1)

    return docker_name


def select_version_to_build(d):
    """
    Use Git to select version to build from tag and branches

    :d: dialog backtitle
    :returns: @todo

    """

    # Get git tag and git branch result
    versions = []
    tags = subprocess.Popen(['git', 'tag'],
                            stdout=subprocess.PIPE)
    branches = subprocess.Popen(['git', 'branch', '--no-color'],
                                stdout=subprocess.PIPE)

    # Add git tags to versions and sort them
    for line in iter(tags.stdout.readline, ''):
        versions.append([line.rstrip(), '', 0])
    versions = sorted(versions, reverse=True)

    # Add git branches to versions
    for line in iter(branches.stdout.readline, ''):
        m = re.search(r"^\*?\s+(.+)", line.rstrip())
        if m is not None:
            versions.append([m.group(1), '', 0])
    versions_len = len(versions)

    # Show menu to select wished packages
    tag = []
    while len(tag) == 0:
        while 1:
            (code, tag) = d.radiolist(text="Select the version to build",
                                      height=versions_len+8, width=60,
                                      list_height=versions_len,
                                      choices=versions,
                                      title='Select the version from Git ' +
                                            'Git tag or branch')
            if handle_exit_code(d, code):
                break
    return(tag)


def show_mss_containers(d):
    """
    Show MySecureShell Docker containers available on the current machine

    :d: dialog backtitle
    :returns: @todo

    """
    try:
        out = os.popen("docker images | awk '/mss_/{print $1}'").read()
        d.msgbox(out, height=15, width=50)
    except:
        print "Can't use docker"
        sys.exit(1)


def delete_mss_containers(d):
    """
    Delete MySecureShell Docker containers available on the current machine

    :d: dialog backtitle
    :returns: @todo

    """
    try:
        make_choices = [("All", "", 0)]
        docker_images = subprocess.Popen(['docker', 'ps', '-a'],
                                         stdout=subprocess.PIPE)
        all_containers = []
        for container in iter(docker_images.stdout.readline, ''):
            m = re.search(r"(mss_\w+)$", container.rstrip())
            if m is not None:
                make_choices.append([m.group(1), '', 0])
                all_containers.append(m.group(1))
        mk_len = len(make_choices)

        # Show menu to select wished packages
        while 1:
            (code, containers) = d.checklist(
                text="Select containers to delete",
                height=mk_len+8, width=60,
                list_height=mk_len,
                choices=make_choices,
                title="Make packages")
            if handle_exit_code(d, code):
                break
    except:
        print "Can't use docker"
        sys.exit(1)

    # Update desired tag
    if containers[0] == 'All':
        containers = all_containers
    containers_to_remove = ' '.join(containers)
    os.system('docker rm -f ' + containers_to_remove)


def main_menu(d):
    """
    Main menu

    :d: dialog backtitle

    :returns: exit code
    """
    while 1:
        while 1:
            (code, tag) = d.menu(
                "Make your choice",
                height=16, width=60, menu_height=10,
                choices=[("1 Update Docker containers",
                         "Get/Update containers"),
                         ("2 Build Docker containers",
                         "Build from Dockerfiles"),
                         ("3 Create packages", "For Debian/Ubuntu/Centos..."),
                         ("4 Make documentations", "In PDF/HTML..."),
                         ("", ""),
                         ("Show installed images", "Only MySecureShell ones"),
                         ("Delete containers", "Only MySecureShell ones"),
                         ("Show vars", "Show current vars"),
                         ("Exit", "")])
            if handle_exit_code(d, code):
                break

        if tag == '1 Update Docker containers':
            update_docker_containers(d)
        elif tag == '2 Build Docker containers':
            build_docker_containers(d)
        elif tag == '3 Create packages':
            create_packages(d)
        elif tag == 'Delete containers':
            delete_mss_containers(d)
        elif tag == 'Show installed images':
            show_mss_containers(d)
        elif tag == 'Show vars':
            show_vars(d)
        elif tag == 'Exit':
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
