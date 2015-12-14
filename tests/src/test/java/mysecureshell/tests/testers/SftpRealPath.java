package mysecureshell.tests.testers;

import mysecureshell.tests.protocol.SshFxpInit;
import mysecureshell.tests.protocol.SshFxpRealPath;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseName;
import mysecureshell.tests.protocol.responses.ResponseStatus;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.junit.Assert;

public class SftpRealPath extends SftpOperation
{
	public String			path = null;
	public String			expectedPath = null;
	
	@Override
	protected void runTest() throws Throwable
	{
		SshFxpInit			init = new SshFxpInit();
		SshFxpRealPath		realPath = new SshFxpRealPath();
		ResponseName		name;
		Response			response;
		
		Assert.assertNotNull(path);
		Assert.assertNotNull(expectedPath);
		init.send(sftp.is, sftp.os, 1);
		
		response = realPath.send(sftp.is, sftp.os, 2, path);
		if (response instanceof ResponseStatus)
		{
			ResponseStatus	status = (ResponseStatus )response;
			
			Assert.fail("Can't do real path on '" + path + "' : " + status.error);
		}
		Assert.assertTrue(response instanceof ResponseName);
		name = (ResponseName) response;
		Assert.assertEquals(1, name.names.length);
		Assert.assertEquals(expectedPath, name.names[0].file);
	}
	
	@Override
	public String toString()
	{
		return new ToStringBuilder(this).toString();
	}
}
