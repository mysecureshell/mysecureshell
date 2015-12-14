package mysecureshell.tests.testers;

import mysecureshell.tests.protocol.SshFxpInit;
import mysecureshell.tests.protocol.SshFxpStatFile;
import mysecureshell.tests.protocol.objects.SftpFileAttributes;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseAttrs;
import mysecureshell.tests.protocol.responses.ResponseStatus;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.junit.Assert;

public class SftpStatFile extends SftpOperation
{
	public String						file = null;
	public Integer						flags = null;
	public SftpFileAttributes			expected;
	public transient SftpFileAttributes	result;
	
	@Override
	protected void runTest() throws Throwable
	{
		SshFxpInit		init = new SshFxpInit();
		SshFxpStatFile	stat = new SshFxpStatFile();
		ResponseAttrs	attrs;
		Response		response;
		
		Assert.assertNotNull(file);
		Assert.assertNotNull(flags);
		init.send(sftp.is, sftp.os, 1);
		response = stat.sendFile(sftp.is, sftp.os, 2, file, flags);
		if (response instanceof ResponseStatus)
		{
			ResponseStatus	status = (ResponseStatus )response;
			
			Assert.fail("Can't open file '" + file + "' : " + status.error);
		}
		Assert.assertTrue(response instanceof ResponseAttrs);
		attrs = (ResponseAttrs )response;
		result = attrs.attributes;
		Assert.assertEquals("Attributes doesn't match", expected, result);
	}
	
	@Override
	protected void tearDown() throws Exception
	{
		expected = result;
		super.tearDown();
	}
	
	@Override
	public String toString()
	{
		return new ToStringBuilder(this).toString();
	}
}
