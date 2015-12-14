package mysecureshell.tests.testers;

import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;

import mysecureshell.tests.protocol.SshFxpClose;
import mysecureshell.tests.protocol.SshFxpInit;
import mysecureshell.tests.protocol.SshFxpOpenFile;
import mysecureshell.tests.protocol.SshFxpWriteFile;
import mysecureshell.tests.protocol.objects.SftpFileAttributes;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseHandle;
import mysecureshell.tests.protocol.responses.ResponseStatus;
import mysecureshell.tests.utils.AssertUtils;
import mysecureshell.tests.utils.PFlags;
import mysecureshell.tests.utils.SftpConnection;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.junit.Assert;

import ch.ethz.ssh2.sftp.ErrorCodes;

public class SftpUploadFile extends SftpOperation
{
	public String			sourceFile = null;
	public String			destinationFile = null;
	public Boolean			isTextFile = false;
	public String			destinationPath = null;
	public Integer			doTimes = 1;
	
	@Override
	protected void runTest() throws Throwable
	{
		SshFxpInit	init = new SshFxpInit();
		
		init.send(sftp.is, sftp.os, 1);
		while (doTimes > 0)
		{
			doTest();
			doTimes--;
		}
	}
	
	private void doTest() throws Throwable
	{
		ByteArrayOutputStream	bSource = new ByteArrayOutputStream();
		FileInputStream			fSource;
		SshFxpOpenFile			open = new SshFxpOpenFile();
		SshFxpWriteFile			write = new SshFxpWriteFile();
		SshFxpClose				close = new SshFxpClose();
		ResponseHandle			handle;
		Response				response;
		byte[]					data = new byte[4096];
		long					offset;
		int						desiredAccess, pflags;
		int						len;
		
		Assert.assertNotNull(sourceFile);
		Assert.assertNotNull(destinationFile);
		fSource = new FileInputStream(sourceFile);
		SftpWho.userIsIdle();

		if (SftpConnection.protocolVersion <= 4)
		{
			desiredAccess = 0;
			pflags = PFlags.SSH2_FXF_CREAT | PFlags.SSH2_FXF_TRUNC | PFlags.SSH2_FXF_WRITE | PFlags.SSH2_FXF_APPEND;
			if (isTextFile)
				pflags |= PFlags.SSH4_FXF_TEXT;
		}
		else //SftpConnection.protocolVersion >= 5
		{
			desiredAccess = 0x00000002 /* ACE4_WRITE_DATA */ | 0x00000004 /* ACE4_APPEND_DATA */;
			pflags = PFlags.SSH5_FXF_TRUNCATE_EXISTING;
			if (isTextFile)
				pflags |= PFlags.SSH5_FXF_ACCESS_TEXT_MODE;
		}
		response = open.send(sftp.is, sftp.os, 2, destinationFile, desiredAccess, pflags, new SftpFileAttributes());
		if (response instanceof ResponseStatus)
		{
			ResponseStatus	status = (ResponseStatus )response;
			
			Assert.fail("Can't open file '" + destinationFile + "' : " + status.error);
		}
		SftpWho.userIsUploading();
		Assert.assertTrue(response instanceof ResponseHandle);
		handle = (ResponseHandle )response;
		offset = 0L;
		while (fSource.available() > 0)
		{
			byte[]	sendData;
			
			len = fSource.read(data);
			bSource.write(data, 0, len);
			if (len == data.length)
				sendData = data;
			else
			{
				sendData = new byte[len];
				System.arraycopy(data, 0, sendData, 0, len);
			}
			response = write.send(sftp.is, sftp.os, 3, handle.getBinaryHandle(), offset, sendData);
			offset += len;
			if (response instanceof ResponseStatus)
			{
				ResponseStatus	status = (ResponseStatus) response;
				
				if (status.status != ErrorCodes.SSH_FX_OK)
					Assert.fail("Can't write file '" + destinationFile + "' : " + status.error);
			}
		}
		fSource.close();
		if (destinationPath != null)
		{
			if (isTextFile)
				AssertUtils.assertSameFileText(bSource, destinationPath);
			else
				AssertUtils.assertSameFileBinary(bSource, destinationPath);
		}
		response = close.send(sftp.is, sftp.os, 4, handle.getBinaryHandle());
		if (response instanceof ResponseStatus)
		{
			ResponseStatus	status = (ResponseStatus )response;
			
			Assert.assertSame("Can't close file '" + destinationFile + "' : " + status.error, ErrorCodes.SSH_FX_OK, status.status);
		}
		else
			Assert.fail();
		SftpWho.userIsIdle();
		Assert.assertNotNull(destinationPath);
	}
	
	@Override
	public String toString()
	{
		return new ToStringBuilder(this).toString();
	}
}
