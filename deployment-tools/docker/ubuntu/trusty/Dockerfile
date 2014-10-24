FROM debian:lucid
MAINTAINER Pierre Mavro <deimos@deimos.fr>

##################
# User Quick Try #
##################

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get -y -o Dpkg::Options::="--force-confdef" \
 -o Dpkg::Options::="--force-confold" install whois procps openssh-server
RUN mkdir /var/run/sshd
RUN echo 'root:root' | chpasswd

#######
# DEV #
#######

RUN apt-get update
RUN apt-get -y install git
RUN git clone https://github.com/mysecureshell/mysecureshell.git
RUN apt-get -y install libacl1-dev libgnutls-dev gcc make
RUN apt-get -y install build-essential fakeroot lintian devscripts debhelper ubuntu-dev-tools \
 cowbuilder autotools-dev
RUN apt-get -y install sphinx-doc sphinx-common python3-sphinx libjs-sphinxdoc \
 python-pip texlive-latex-base texlive-latex-recommended texlive-latex-extra texlive-fonts-recommended
RUN apt-get clean
RUN pip install sphinx_rtd_theme

# Start SSHd
EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
