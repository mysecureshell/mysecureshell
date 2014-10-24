FROM centos:centos7
MAINTAINER Pierre Mavro <deimos@deimos.fr>

##################
# User Quick Try #
##################

RUN yum -y install whois procps openssh-server
RUN mkdir /var/run/sshd
RUN echo 'root:root' | chpasswd

#######
# DEV #
#######

RUN yum -y install git
RUN git clone https://github.com/mysecureshell/mysecureshell.git
RUN yum -y install libacl1-dev libgnutls28-dev gcc make
RUN yum -y install fakeroot devscripts autotools-dev
RUN yum -y install sphinx-doc sphinx-common python3-sphinx libjs-sphinxdoc \
 python-pip texlive-latex-base texlive-latex-recommended texlive-latex-extra texlive-fonts-recommended
RUN pip install sphinx_rtd_theme

# Start SSHd
EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
