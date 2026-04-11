FROM debian:bookworm

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        gcc make libacl1-dev acl openssh-server sshpass && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

COPY . /src/mysecureshell
WORKDIR /src/mysecureshell

RUN ./configure --prefix=/usr && make all && make install && \
    chmod 4755 /usr/bin/mysecureshell

# Create test users
RUN addgroup --gid 3010 aclgroup && \
    adduser --home /home/testuser --shell /usr/bin/mysecureshell \
        --uid 4010 --gid 3010 --disabled-password --gecos "" testuser && \
    echo 'testuser:testpass' | chpasswd && \
    adduser --home /home/acluser --shell /bin/bash \
        --uid 4011 --gid 3010 --disabled-password --gecos "" acluser && \
    echo 'acluser:aclpass' | chpasswd

# Configure sshd for password auth
RUN mkdir -p /var/run/sshd && \
    sed -i 's/^#PasswordAuthentication.*/PasswordAuthentication yes/' /etc/ssh/sshd_config && \
    sed -i 's/^#PermitRootLogin.*/PermitRootLogin no/' /etc/ssh/sshd_config && \
    ssh-keygen -A

# Minimal sftp_config
RUN printf '<Default>\n    LogLevel 5\n</Default>\n' > /etc/ssh/sftp_config

COPY tests/docker-acl-test/run-test.sh /run-test.sh
RUN chmod +x /run-test.sh

CMD ["/run-test.sh"]
