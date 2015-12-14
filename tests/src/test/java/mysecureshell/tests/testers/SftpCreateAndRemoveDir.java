package mysecureshell.tests.testers;

import mysecureshell.tests.protocol.SshFxpInit;
import mysecureshell.tests.protocol.SshFxpMakeDir;
import mysecureshell.tests.protocol.SshFxpRemoveDir;
import mysecureshell.tests.protocol.SshFxpStatFile;
import mysecureshell.tests.protocol.objects.SftpFileAttributes;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseAttrs;
import mysecureshell.tests.protocol.responses.ResponseStatus;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.junit.Assert;

import ch.ethz.ssh2.sftp.ErrorCodes;

public class SftpCreateAndRemoveDir extends SftpOperation
{
	public String						path = null;
	public SftpFileAttributes			attributes = null;
	public SftpFileAttributes			afterMkDirAttributes = null;
	transient public SftpFileAttributes	resultAttributes = null;
	
	@Override
	protected void runTest() throws Throwable
	{
		SshFxpInit				init = new SshFxpInit();
		SshFxpMakeDir			mkDir = new SshFxpMakeDir();
		SshFxpRemoveDir			rmDir = new SshFxpRemoveDir();
		SshFxpStatFile			stat = new SshFxpStatFile();
		ResponseStatus			status;
		Response				response;
		
		Assert.assertNotNull(path);
		Assert.assertNotNull(attributes);
		init.send(sftp.is, sftp.os, 1);
		status = mkDir.send(sftp.is, sftp.os, 2, path, attributes);
		if (status.status != ErrorCodes.SSH_FX_OK)
			Assert.fail("Can't create dir '" + path + "' : " + status.error);
		response = stat.sendFile(sftp.is, sftp.os, 3, path, attributes.flags);
		if (response instanceof ResponseStatus)
		{
			status = (ResponseStatus) response;
			Assert.fail("Can't stat dir '" + path + "' : " + status.error);
		}
		else
		{
			ResponseAttrs	attrs = (ResponseAttrs) response;
			
			resultAttributes = attrs.attributes;
			if (afterMkDirAttributes != null)
				Assert.assertEquals(afterMkDirAttributes, resultAttributes);
			else
				Assert.assertEquals(attributes, resultAttributes);
		}
		status = rmDir.send(sftp.is, sftp.os, 4, path);
		if (status.status != ErrorCodes.SSH_FX_OK)
			Assert.fail("Can't create dir '" + path + "' : " + status.error);
	}
	
	@Override
	protected void tearDown() throws Exception
	{
		attributes = resultAttributes;
		super.tearDown();
	}
	
	@Override
	public String toString()
	{
		return new ToStringBuilder(this).toString();
	}
}
