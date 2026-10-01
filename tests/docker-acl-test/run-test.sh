#!/bin/bash
set -e

echo "=== ACL Mask Preservation Test ==="

# Start sshd
/usr/sbin/sshd

# Set up test directory with default ACLs
TEST_DIR="/home/testuser/acltest"
mkdir -p "$TEST_DIR"
chown testuser:aclgroup "$TEST_DIR"
chmod 0755 "$TEST_DIR"

# Grant acluser rwx via default ACL so new files inherit it
setfacl -d -m u:acluser:rwx "$TEST_DIR"
setfacl -m u:acluser:rwx "$TEST_DIR"

echo "--- Directory ACL before upload ---"
getfacl "$TEST_DIR"

# Create a local file to upload
UPLOAD_FILE="/tmp/testfile.txt"
echo "Hello ACL test" > "$UPLOAD_FILE"

# Upload file via SFTP through MySecureShell
echo "--- Uploading file via SFTP ---"
sshpass -p 'testpass' sftp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
    testuser@localhost <<SFTP_EOF
cd acltest
put $UPLOAD_FILE uploaded.txt
SFTP_EOF

UPLOADED="$TEST_DIR/uploaded.txt"

if [ ! -f "$UPLOADED" ]; then
    echo "FAIL: File was not uploaded"
    exit 1
fi

echo "--- Uploaded file ACL ---"
getfacl "$UPLOADED"

# Check that acluser's ACL entry still has write permission
# getfacl output for named user looks like: user:acluser:rwx
ACL_LINE=$(getfacl "$UPLOADED" 2>/dev/null | grep "^user:acluser:")

if [ -z "$ACL_LINE" ]; then
    echo "FAIL: No ACL entry found for acluser"
    exit 1
fi

echo "ACL entry: $ACL_LINE"

# Check the entry has write permission (rwx or rw-)
if echo "$ACL_LINE" | grep -q "user:acluser:rw"; then
    echo "PASS: ACL write permission preserved for acluser"
else
    echo "FAIL: ACL write permission lost for acluser"
    echo "Expected user:acluser:rw[x-] but got: $ACL_LINE"

    # Also check effective permissions
    EFFECTIVE=$(getfacl "$UPLOADED" 2>/dev/null | grep "^mask::")
    echo "Mask entry: $EFFECTIVE"
    exit 1
fi

# Also verify the mask allows write
MASK_LINE=$(getfacl "$UPLOADED" 2>/dev/null | grep "^mask::")
echo "Mask entry: $MASK_LINE"

if echo "$MASK_LINE" | grep -q "mask::rw"; then
    echo "PASS: ACL mask preserves write permission"
else
    echo "FAIL: ACL mask does not include write permission"
    echo "Got: $MASK_LINE"
    exit 1
fi

echo ""
echo "=== All ACL tests passed ==="
