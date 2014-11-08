#!/bin/sh
### BEGIN INIT INFO
# Provides:          mysecureshell
# Required-Start:    $local_fs $network $remote_fs $syslog
# Required-Stop:     $local_fs $network $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: MySecureShell SFTP Server
# Description:       MySecureShell SFTP Server
### END INIT INFO

# Author: MySecureShell Team <teka2nerdman@users.sourceforge.net>

# Do NOT "set -e"

# PATH should only include /usr/* if it runs after the mountnfs.sh script
PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
DESC="MySecureShell SFTP Server"
NAME=mysecureshell
DAEMON=/usr/bin/mysecureshell
DAEMON_ARGS=""
PIDFILE=/var/run/$NAME.pid
SCRIPTNAME=/etc/init.d/$NAME

# Exit if the package is not installed
[ -x "$DAEMON" ] || exit 0

# Read configuration variable file if it is present
[ -r /etc/default/$NAME ] && . /etc/default/$NAME

# Load the VERBOSE setting and other rcS variables
. /lib/init/vars.sh

# Define LSB log_* functions.
# Depend on lsb-base (>= 3.2-14) to ensure that this file is present
# and status_of_proc is working.
. /lib/lsb/init-functions

do_start () {
    echo -n "Starting $DESC: "
    sftp-state start > /dev/null
    if [ $(stat --format='%a' $DAEMON) -eq 755 ] ; then
        echo "$NAME is now online with restricted features"
        echo "Note: To enable all features you have to change mysecureshell binary rights to 4755"
    else
        echo "$NAME is now online with full features"
    fi
}

do_stop () {
    echo -n "Stopping $DESC: "
    sftp-state fullstop -yes > /dev/null
    echo "$NAME is now offline"
}

#
# Function that starts the daemon/service
#
case "$1" in
  start)
    do_start
    ;;
  stop)
    do_stop
    ;;
  restart|force-reload)
    do_stop
    echo "$NAME is now offline --> please wait while restarting..."
    sleep 1
    do_start
    ;;
  status)
    sftp-state
    ;;
  *)
    N=/etc/init.d/$NAME
    echo "Usage: $N {start|stop|restart|status|force-reload}" >&2
    exit 1
    ;;
esac

exit 0

