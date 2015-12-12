package mysecureshell.tests.testers;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import mysecureshell.tests.protocol.SshFxpInit;
import mysecureshell.tests.protocol.SshFxpReadLink;
import mysecureshell.tests.protocol.SshFxpRemoveFile;
import mysecureshell.tests.protocol.SshFxpSymLink;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseName;
import mysecureshell.tests.protocol.responses.ResponseStatus;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.junit.Assert;

import ch.ethz.ssh2.sftp.ErrorCodes;

@XmlRootElement(name="testers.SftpCreateAndRemoveSymLink")
public class SftpCreateAndRemoveSymLink extends SftpOperation
{
	@XmlElement
	public String				linkPath = null;
	
	@XmlElement
	public String				targetPath = null;
	
	@XmlElement
	public Boolean				remove = Boolean.TRUE;
	
	@Override
	protected void runTest() throws Throwable
	{
		SshFxpInit			init = new SshFxpInit();
		SshFxpSymLink		ln = new SshFxpSymLink();
		SshFxpReadLink		readLink = new SshFxpReadLink();
		SshFxpRemoveFile	rm = new SshFxpRemoveFile();
		ResponseStatus		status;
		ResponseName		name;
		Response			response;
		
		Assert.assertNotNull(linkPath);
		Assert.assertNotNull(targetPath);
		init.send(sftp.is, sftp.os, 1);
		status = ln.send(sftp.is, sftp.os, 2, linkPath, targetPath);
		if (status.status != ErrorCodes.SSH_FX_OK)
			Assert.fail("Can't create symlink '" + linkPath + "' -> '" + targetPath +"' : " + status.error);
		response = readLink.send(sftp.is, sftp.os, 3, linkPath);
		if (response instanceof ResponseStatus)
		{
			status = (ResponseStatus) response;
			
			Assert.fail("Can't readlink symlink '" + linkPath +"' : " + status.error);
		}
		Assert.assertTrue(response instanceof ResponseName);
		name = (ResponseName) response;
		Assert.assertEquals(1, name.names.length);
		Assert.assertEquals(targetPath, name.names[0].file);
		if (Boolean.TRUE.equals(remove))
		{
			status = rm.send(sftp.is, sftp.os, 4, linkPath);
			if (status.status != ErrorCodes.SSH_FX_OK)
				Assert.fail("Can't remove symlink '" + linkPath +"' : " + status.error);
		}
	}
	
	@Override
	public String toString()
	{
		return new ToStringBuilder(this).toString();
	}
}
