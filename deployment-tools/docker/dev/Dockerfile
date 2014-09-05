FROM debian:stable
MAINTAINER Pierre Mavro <deimos@deimos.fr>

##################
# User Quick Try #
##################

RUN echo 'deb http://mysecureshell.free.fr/repository/index.php/debian/7.1 testing main' \
> /etc/apt/sources.list.d/mysecureshell.list
RUN echo 'deb-src http://mysecureshell.free.fr/repository/index.php/debian/7.1 testing main' \
>> /etc/apt/sources.list.d/mysecureshell.list
RUN gpg --keyserver hkp://pool.sks-keyservers.net --recv-keys E328F22B
RUN gpg --export E328F22B | apt-key add -
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get -y -o Dpkg::Options::="--force-confdef" \
 -o Dpkg::Options::="--force-confold" install mysecureshell whois procps openssh-server
RUN mkdir /var/run/sshd
RUN pass=$(mkpasswd -m sha-512 -s mssuser) && useradd -m -s /bin/MySecureShell -p $pass mssuser
RUN echo 'root:root' | chpasswd

#######
# DEV #
#######

RUN apt-get update
RUN apt-get -y install git
RUN git clone https://github.com/mysecureshell/mysecureshell.git
RUN apt-get -y install libacl1-dev libgnutls-dev gcc make
RUN apt-get -y install sphinx-doc sphinx-common python3-sphinx libjs-sphinxdoc \
 python-pip texlive-latex-base texlive-latex-recommended texlive-latex-extra texlive-fonts-recommended
RUN pip install sphinx_rtd_theme

# Start SSHd
EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
