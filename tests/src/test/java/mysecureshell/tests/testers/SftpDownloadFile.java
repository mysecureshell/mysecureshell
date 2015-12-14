package mysecureshell.tests.testers;

import java.io.ByteArrayOutputStream;
import java.util.logging.Logger;

import mysecureshell.tests.protocol.SshFxpClose;
import mysecureshell.tests.protocol.SshFxpInit;
import mysecureshell.tests.protocol.SshFxpOpenFile;
import mysecureshell.tests.protocol.SshFxpReadFile;
import mysecureshell.tests.protocol.objects.SftpFileAttributes;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseData;
import mysecureshell.tests.protocol.responses.ResponseHandle;
import mysecureshell.tests.protocol.responses.ResponseStatus;
import mysecureshell.tests.utils.AssertUtils;
import mysecureshell.tests.utils.PFlags;
import mysecureshell.tests.utils.SftpConnection;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.junit.Assert;

import ch.ethz.ssh2.sftp.ErrorCodes;

public class SftpDownloadFile extends SftpOperation
{
	private static Logger	log = Logger.getLogger("SftpDownloadFile");
	public String			file = null;
	public Boolean			isTextFile = false;
	public String			originalPath = null;
	
	@Override
	protected void runTest() throws Throwable
	{
		if (mustFail)
		{
			try
			{
				doTest();
				Assert.fail("Should not success");
			}
			catch (AssertionError e)
			{
				log.info("Transfert of '" + file + "' has failed, but it's okay !");
			}
		}
		else
			doTest();
	}
	
	private void doTest() throws Throwable
	{
		ByteArrayOutputStream	bos = new ByteArrayOutputStream();
		SshFxpInit				init = new SshFxpInit();
		SshFxpOpenFile			open = new SshFxpOpenFile();
		SshFxpReadFile			read = new SshFxpReadFile();
		SshFxpClose				close = new SshFxpClose();
		ResponseHandle			handle;
		Response				response;
		boolean					continueRead = true;
		long					offset = 0;
		int						desiredAccess, pflags;
		
		Assert.assertNotNull(file);
		init.send(sftp.is, sftp.os, 1);
		SftpWho.userIsIdle();

		if (SftpConnection.protocolVersion <= 4)
		{
			desiredAccess = 0;
			pflags = PFlags.SSH2_FXF_READ;
			if (isTextFile)
				pflags |= PFlags.SSH4_FXF_TEXT;
		}
		else //SftpConnection.protocolVersion >= 5
		{
			desiredAccess = 0x00000001 /* ACE4_READ_DATA */;
			pflags = PFlags.SSH5_FXF_OPEN_EXISTING;
			if (isTextFile)
				pflags |= PFlags.SSH5_FXF_ACCESS_TEXT_MODE;
		}
		response = open.send(sftp.is, sftp.os, 2, file, desiredAccess, pflags, new SftpFileAttributes());
		if (response instanceof ResponseStatus)
		{
			ResponseStatus	status = (ResponseStatus )response;
			
			Assert.fail("Can't open file '" + file + "' : " + status.error);
		}
		SftpWho.userIsDownloading();
		Assert.assertTrue(response instanceof ResponseHandle);
		handle = (ResponseHandle )response;
		do
		{
			response = read.send(sftp.is, sftp.os, 3, handle.getBinaryHandle(), offset, 4096);
			if (response instanceof ResponseStatus)
			{
				ResponseStatus	status = (ResponseStatus) response;
				
				continueRead = false;
				if (status.status != ErrorCodes.SSH_FX_EOF)
					Assert.fail("Can't read file '" + file + "' : " + status.error);
			}
			else
			{
				ResponseData	data = (ResponseData) response;
				
				Assert.assertTrue(data.getDataLength() > 0);
				bos.write(data.data);
				offset += data.getDataLength();
			}
		}
		while (continueRead);
		log.info("Downloaded: " + bos.size());
		if (isTextFile)
			System.out.println("Content of file: [[[" + bos.toString() + "]]]");
		if (originalPath != null)
		{
			if (isTextFile)
				AssertUtils.assertSameFileText(originalPath, bos);
			else
				AssertUtils.assertSameFileBinary(originalPath, bos);
		}
		response = close.send(sftp.is, sftp.os, 4, handle.getBinaryHandle());
		if (response instanceof ResponseStatus)
		{
			ResponseStatus	status = (ResponseStatus )response;
			
			Assert.assertSame("Can't close file '" + file + "' : " + status.error, ErrorCodes.SSH_FX_OK, status.status);
		}
		else
			Assert.fail();
		SftpWho.userIsIdle();
		Assert.assertNotNull(originalPath);
	}
	
	@Override
	public String toString()
	{
		return new ToStringBuilder(this).toString();
	}
}
