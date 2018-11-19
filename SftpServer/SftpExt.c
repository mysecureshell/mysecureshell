/*
 MySecureShell permit to add restriction to modified sftp-server
 when using MySecureShell as shell.
 Copyright (C) 2007-2014 MySecureShell Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation (version 2)

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "../config.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Buffer.h"
#include "Encoding.h"
#include "FileSystem.h"
#include "Handle.h"
#include "Log.h"
#include "Send.h"
#include "SftpExt.h"
#include "SftpServer.h"
#include "Util.h"
#include "../Core/security.h"

#ifdef MSSEXT_DISKUSAGE
void DoExtDiskSpace(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
{
	struct STATFS stfs;
	tFSPath	*realPath;
	char *path;

	path = convertFromUtf8(BufferGetString(bIn), 1);
	realPath = FSCheckPath(path);
	DEBUG((MYLOG_DEBUG, "[DoExtDiskSpaceOpenSSH]Path: %s", path));
	if (realPath != NULL && !HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_STATSFS))
	{
		DEBUG((MYLOG_DEBUG, "[DoExtDiskSpaceOpenSSH]Realpath: %s", realPath->realPath));
		if (STATFS(realPath->realPath, &stfs) == 0)
		{
			tBuffer *b;

			b = BufferNew();
			BufferPutInt8(b, SSH2_FXP_EXTENDED_REPLY);
			BufferPutInt32(b, id);
			BufferPutInt64(b, (u_int64_t) stfs.f_blocks * (u_int64_t) stfs.f_bsize);
			BufferPutInt64(b, (u_int64_t) stfs.f_bfree * (u_int64_t) stfs.f_bsize);
			BufferPutInt64(b, 0);
			BufferPutInt64(b, (u_int64_t) stfs.f_bavail * (u_int64_t) stfs.f_bsize);
			BufferPutInt32(b, stfs.f_bsize);
			BufferPutPacket(bOut, b);
			BufferDelete(b);
		}
		else
			SendStatus(bOut, id, errnoToPortable(errno));
	}
	else
		SendStatus(bOut, id, SSH2_FX_PERMISSION_DENIED);
	FSDestroyPath(realPath);
	free(path);
}
#endif //MSSEXT_DISKUSAGE
#ifdef MSSEXT_DISKUSAGE_SSH
static void DoExtDiskSpaceOpenSSH_Path(tBuffer *bOut, u_int32_t id, const char *path)
{
	struct STATFS stfs;
	tFSPath	*realPath;

	DEBUG((MYLOG_DEBUG, "[DoExtDiskSpaceOpenSSH_Path]Path: %s", path));
	realPath = FSCheckPath(path);
	if (realPath != NULL && !HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_STATSFS))
	{
		DEBUG((MYLOG_DEBUG, "[DoExtDiskSpaceOpenSSH_Path]Realpath: %s", realPath->realPath));
		if (STATFS(realPath->realPath, &stfs) == 0)
		{
			tBuffer *b;

			b = BufferNew();
			BufferPutInt8(b, SSH2_FXP_EXTENDED_REPLY);
			BufferPutInt32(b, id);
			BufferPutInt64(b, stfs.f_bsize); /* file system block size */
			BufferPutInt64(b, stfs.f_frsize); /* fundamental fs block size */
			BufferPutInt64(b, stfs.f_blocks); /* number of blocks (unit f_frsize) */
			BufferPutInt64(b, stfs.f_bfree); /* free blocks in file system */
			BufferPutInt64(b, stfs.f_bavail); /* free blocks for non-root */
			BufferPutInt64(b, stfs.f_files); /* total file inodes */
			BufferPutInt64(b, stfs.f_ffree); /* free file inodes */
			BufferPutInt64(b, stfs.f_favail); /* free file inodes for to non-root */
			BufferPutInt64(b, stfs.f_fsid); /* file system id */
			BufferPutInt64(b, stfs.f_flag); /* bit mask of f_flag values */
			BufferPutInt64(b, stfs.f_namemax); /* maximum filename length */
			BufferPutPacket(bOut, b);
			BufferDelete(b);
		}
		else
		{
			DEBUG((MYLOG_DEBUG, "[DoExtDiskSpaceOpenSSH_Path]error: %s", strerror(errno)));
			SendStatus(bOut, id, errnoToPortable(errno));
		}
	}
	else
	{
		DEBUG((MYLOG_DEBUG, "[DoExtDiskSpaceOpenSSH_Path]FSCheckPath failed"));
		SendStatus(bOut, id, SSH2_FX_PERMISSION_DENIED);
	}
	FSDestroyPath(realPath);
}

void DoExtDiskSpaceOpenSSH_Handle(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
{
	tHandle *hdl;

	if ((hdl = HandleGetDir(BufferGetHandle(bIn))) != NULL)
		DoExtDiskSpaceOpenSSH_Path(bOut, id, hdl->path);
	else
		SendStatus(bOut, id, SSH4_FX_INVALID_HANDLE);
}

void DoExtDiskSpaceOpenSSH_Name(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
{
	char *path;

	path = BufferGetString(bIn);
	DoExtDiskSpaceOpenSSH_Path(bOut, id, path);
	free(path);
}
#endif //MSSEXT_DISKUSAGE_SSH
#ifdef MSSEXT_FILE_HASHING
#include <gnutls/gnutls.h>
#include <gnutls/crypto.h>

static void DoExtFileHashing_FD(tBuffer *bIn, tBuffer *bOut, u_int32_t id, int fd)
{
	gnutls_digest_algorithm_t gnuTlsAlgo = GNUTLS_DIG_UNKNOWN;
	u_int64_t offset, length;
	u_int32_t blockSize;
	tBuffer *b = NULL;
	char *gnuKey = NULL;
	char *algo;

	algo = BufferGetString(bIn);
	offset = BufferGetInt64(bIn);
	length = BufferGetInt64(bIn);
	blockSize = BufferGetInt32(bIn);
	if (lseek(fd, offset, SEEK_SET) == -1)
	{
		SendStatus(bOut, id, errnoToPortable(errno));
		DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Error lseek1"));
		goto endOfFileHashing;
	}
	if (length == 0)//read the file to the end
	{
		u_int64_t endOfFile;

		if ((endOfFile = lseek(fd, 0, SEEK_END)) == -1)
		{
			SendStatus(bOut, id, errnoToPortable(errno));
			DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Error lseek2"));
			goto endOfFileHashing;
		}
		length = endOfFile - offset;
		if (lseek(fd, offset, SEEK_SET) == -1)
		{
			SendStatus(bOut, id, errnoToPortable(errno));
			DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Error lseek3"));
			goto endOfFileHashing;
		}
	}
	if (blockSize == 0)//read length in one time
		blockSize = length;
	DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Algo:%s Fd:%i Offset:%llu Length:%llu BlockSize:%i",
					algo, fd, offset, length, blockSize));
	if (strcasecmp("md2", algo) == 0)
		gnuTlsAlgo = GNUTLS_DIG_MD2;
	else if (strcasecmp("md5", algo) == 0)
		gnuTlsAlgo = GNUTLS_DIG_MD5;
	else if (strcasecmp("sha1", algo) == 0)
		gnuTlsAlgo = GNUTLS_DIG_SHA1;
	else if (strcasecmp("sha224", algo) == 0)
		gnuTlsAlgo = GNUTLS_DIG_SHA224;
	else if (strcasecmp("sha256", algo) == 0)
		gnuTlsAlgo = GNUTLS_DIG_SHA256;
	else if (strcasecmp("sha384", algo) == 0)
		gnuTlsAlgo = GNUTLS_DIG_SHA384;
	else if (strcasecmp("sha512", algo) == 0)
		gnuTlsAlgo = GNUTLS_DIG_SHA512;
	if (gnuTlsAlgo != GNUTLS_DIG_UNKNOWN)
	{
		gnutls_hash_hd_t dig;
		size_t keySize = gnutls_hash_get_len(gnuTlsAlgo);
		char data[SSH2_READ_HASH];
		int inError = 0;
		int gnulTlsError;

		b = BufferNew();
		BufferPutInt8FAST(b, SSH2_FXP_EXTENDED_REPLY);
		BufferPutInt32(b, id);
		BufferPutString(b, algo);
		gnuKey = calloc(1, keySize);
		if (gnuKey == NULL)
			goto endOfFileHashing;
		if ((gnulTlsError = gnutls_hash_init(&dig, gnuTlsAlgo)) == 0)
		{
			while (length > 0)
			{
				u_int32_t r, off, len;

				length = (length > (u_int64_t) blockSize) ? length - (u_int64_t) blockSize : 0;
				off = blockSize;
				len = sizeof(data);
				DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Read:%i Rest:%llu", len, length));
				while ((r = read(fd, data, len)) > 0)
				{
					DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Compute block (%u/%u %u)", len, r, off));
					if ((gnulTlsError = gnutls_hash(dig, data, r)) != 0)
					{
						DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Error gnutls_hmac [error: %i]", gnulTlsError));
						inError = 1;
						break;
					}
					off -= r;
					if (off < sizeof(data))
						len = off;
					if (off == 0)
						break;
				}
			}
		}
		else
		{
			DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Error gnutls_hash_init [keySize: %li] [error: %i]", keySize, gnulTlsError));
			inError = 1;
		}
		if (inError == 0)
		{
			DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Compute key... [keySize: %li][keyPointer: %p]", keySize, gnuKey));
			gnutls_hash_deinit(dig, gnuKey);
			DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Hash: %X%X%X ...", gnuKey[0], gnuKey[1], gnuKey[2]));
			BufferPutRawData(b, gnuKey, keySize);
			BufferPutPacket(bOut, b);
		}
		else
			SendStatus(bOut, id, SSH2_FX_FAILURE);
	}
	else
	{
		DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]No algorithm: %s", algo));
		SendStatus(bOut, id, SSH2_FX_OP_UNSUPPORTED);
	}
	endOfFileHashing: DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]End"));
	if (gnuKey != NULL)
		free(gnuKey);
	if (b != NULL)
		BufferDelete(b);
	free(algo);
}

void DoExtFileHashing_Handle(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
{
	tHandle *hdl;

	DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_Handle]..."));
	if ((hdl = HandleGetFile(BufferGetHandle(bIn))) != NULL)
		DoExtFileHashing_FD(bIn, bOut, id, hdl->fd);
	else
		SendStatus(bOut, id, SSH4_FX_INVALID_HANDLE);
}

void DoExtFileHashing_Name(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
{
	char *file = BufferGetString(bIn);
	int status, fd;

	status = FSOpenFile(file, &fd, O_RDONLY, 0, NULL);
	DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_Name]File: %s Status: %i", file, status));
	if (status == SSH2_FX_OK)
	{
		DoExtFileHashing_FD(bIn, bOut, id, fd);
		xclose(fd);
	}
	else
		SendStatus(bOut, id, status);
	free(file);
}
#endif //MSSEXT_FILE_HASHING

void DoExtHardLink(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
{
	char *link, *target;
	int status = SSH2_FX_OK;

	link = convertFromUtf8(BufferGetString(bIn), 1);
	target = convertFromUtf8(BufferGetString(bIn), 1);
	if (HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_SYMLINK))
	{
		DEBUG((MYLOG_DEBUG, "[DoExtHardLink]Disabled by conf."));
		status = SSH2_FX_PERMISSION_DENIED;
	}
	else
	{
		status = FSHardlink(target, link);
		DEBUG((MYLOG_DEBUG, "[DoExtHardLink]link:'%s' target:'%s' -> %i", link, target, status));
	}
	SendStatus(bOut, id, status);
	free(target);
	free(link);
}
