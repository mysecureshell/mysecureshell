# ACL Mask Preservation Test

Tests that POSIX ACL masks are preserved when files are uploaded via SFTP through MySecureShell (see [#69](https://github.com/mysecureshell/mysecureshell/issues/69)).

## Usage

From the repository root:

```bash
docker build -f tests/docker-acl-test/Dockerfile -t mss-acl-test .
docker run --rm mss-acl-test
```

## What it tests

1. Creates a directory with default ACLs granting a secondary user `rwx`
2. Uploads a file via SFTP through MySecureShell
3. Verifies the uploaded file's ACL entry retains write permission
4. Verifies the ACL mask includes write permission (no `#effective:r-x` degradation)
