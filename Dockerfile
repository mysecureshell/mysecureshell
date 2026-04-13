# =============================================================================
# Stage 1: Build MySecureShell from source
# =============================================================================
FROM debian:bookworm-slim AS builder

ARG MSS_VERSION=""

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        gcc \
        make \
        libc6-dev \
        libacl1-dev \
        libgnutls28-dev \
        curl \
        ca-certificates \
        jq && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /build

# If MSS_VERSION is empty, auto-detect latest release from GitHub API.
# Then download and extract the source tarball.
RUN set -eux; \
    VERSION="${MSS_VERSION}"; \
    if [ -z "$VERSION" ]; then \
        VERSION=$(curl -fsSL https://api.github.com/repos/mysecureshell/mysecureshell/releases/latest | jq -r '.tag_name'); \
    fi; \
    echo "Building MySecureShell $VERSION"; \
    curl -fsSL "https://github.com/mysecureshell/mysecureshell/archive/refs/tags/${VERSION}.tar.gz" \
        | tar xz --strip-components=1

RUN ./configure --prefix=/usr && \
    make all

# =============================================================================
# Stage 2: Runtime image
# =============================================================================
FROM debian:bookworm-slim

LABEL maintainer="Pierre Mavro <deimos@deimos.fr>" \
      org.opencontainers.image.title="MySecureShell" \
      org.opencontainers.image.description="SFTP server with ACL based on OpenSSH" \
      org.opencontainers.image.url="https://github.com/mysecureshell/mysecureshell" \
      org.opencontainers.image.source="https://github.com/mysecureshell/mysecureshell"

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        openssh-server \
        libacl1 \
        libgnutls30 \
        procps \
        whois && \
    rm -rf /var/lib/apt/lists/* && \
    mkdir -p /var/run/sshd && \
    ssh-keygen -A

# Copy compiled binaries directly from builder (bypass install.sh which
# has DESTDIR path mismatches between ETCDIR and MSS_CONF in Docker)
COPY --from=builder /build/mysecureshell           /usr/bin/mysecureshell
COPY --from=builder /build/utils/sftp-who          /usr/bin/sftp-who
COPY --from=builder /build/utils/sftp-state        /usr/bin/sftp-state
COPY --from=builder /build/utils/sftp-admin        /usr/bin/sftp-admin
COPY --from=builder /build/utils/sftp-kill         /usr/bin/sftp-kill
COPY --from=builder /build/utils/sftp-verif        /usr/bin/sftp-verif
COPY --from=builder /build/utils/sftp-user         /usr/bin/sftp-user
COPY --from=builder /build/sftp_config             /etc/ssh/sftp_config
COPY --from=builder /build/man/                    /usr/share/man/

# Set permissions, register shell, create demo user (password: mssuser)
RUN chmod 4755 /usr/bin/mysecureshell && \
    chmod 755 /usr/bin/sftp-who /usr/bin/sftp-verif /usr/bin/sftp-user && \
    chmod 700 /usr/bin/sftp-state /usr/bin/sftp-kill /usr/bin/sftp-admin && \
    echo '/usr/bin/mysecureshell' >> /etc/shells && \
    pass=$(mkpasswd -m sha-512 mssuser) && \
    useradd -m -s /usr/bin/mysecureshell -p "$pass" mssuser && \
    echo 'root:root' | chpasswd

EXPOSE 22

CMD ["/usr/sbin/sshd", "-D"]
