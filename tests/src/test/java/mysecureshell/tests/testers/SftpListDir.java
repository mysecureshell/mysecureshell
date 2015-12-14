package mysecureshell.tests.testers;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import mysecureshell.tests.protocol.SshFxpClose;
import mysecureshell.tests.protocol.SshFxpInit;
import mysecureshell.tests.protocol.SshFxpOpenDir;
import mysecureshell.tests.protocol.SshFxpReadDir;
import mysecureshell.tests.protocol.objects.SftpFile;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseHandle;
import mysecureshell.tests.protocol.responses.ResponseName;
import mysecureshell.tests.protocol.responses.ResponseStatus;
import mysecureshell.tests.utils.FilesUtil;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.junit.Assert;

import ch.ethz.ssh2.sftp.ErrorCodes;

public class SftpListDir extends SftpOperation
{
	public String					path = null;
	public SftpFile[]				wantedFiles = null;
	private transient SftpFile[]	resultFiles = null;
	
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
				;
			}
		}
		else
			doTest();
	}
	
	private void doTest() throws Throwable
	{
		List<SftpFile>	files = new ArrayList<SftpFile>();
		SshFxpInit 		init = new SshFxpInit();
		SshFxpOpenDir 	open = new SshFxpOpenDir();
		SshFxpReadDir	read = new SshFxpReadDir();
		SshFxpClose 	close = new SshFxpClose();
		ResponseHandle	handle;
		Response		response;
		boolean			continueRead = true;

		Assert.assertNotNull(path);
		init.send(sftp.is, sftp.os, 1);
		response = open.send(sftp.is, sftp.os, 2, path);
		if (response instanceof ResponseStatus)
		{
			ResponseStatus status = (ResponseStatus) response;

			Assert.fail("Can't open dir '" + path + "' : " + status.error);
		}
		Assert.assertTrue(response instanceof ResponseHandle);
		handle = (ResponseHandle) response;
		do
		{
			response = read.send(sftp.is, sftp.os, 3, handle.getBinaryHandle());
			if (response instanceof ResponseStatus)
			{
				ResponseStatus status = (ResponseStatus) response;

				continueRead = false;
				if (status.status != ErrorCodes.SSH_FX_EOF)
					Assert.fail("Can't read dir '" + path + "' : " + status.error);
			}
			else
			{
				ResponseName names = (ResponseName) response;

				for (SftpFile pair : names.names)
				{
					Assert.assertNotNull(pair.file);
					files.add(pair);
				}
			}
		}
		while (continueRead);
		resultFiles = files.toArray(new SftpFile[files.size()]);
		FilesUtil.sort(resultFiles);
		if (wantedFiles != null)
			Assert.assertArrayEquals("wanted=" + Arrays.toString(wantedFiles) + " result=" + Arrays.toString(resultFiles), wantedFiles, resultFiles);
		response = close.send(sftp.is, sftp.os, 4, handle.getBinaryHandle());
		if (response instanceof ResponseStatus)
		{
			ResponseStatus status = (ResponseStatus) response;

			Assert.assertSame("Can't close dir '" + path + "' : " + status.error, ErrorCodes.SSH_FX_OK, status.status);
		}
		else
			Assert.fail();
		if (doDumpResult)
			wantedFiles = files.toArray(new SftpFile[files.size()]);
		Assert.assertNotNull(resultFiles);
	}
	
	@Override
	protected void tearDown() throws Exception
	{
		wantedFiles = resultFiles;
		super.tearDown();
	}
	
	@Override
	public String toString()
	{
		return new ToStringBuilder(this).toString();
	}
}
