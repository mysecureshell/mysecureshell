package mysecureshell.tests.testers;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import mysecureshell.tests.protocol.SshFxpInit;
import mysecureshell.tests.protocol.SshFxpRename;
import mysecureshell.tests.protocol.responses.ResponseStatus;
import mysecureshell.tests.utils.SftpConnection;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.junit.Assert;

import ch.ethz.ssh2.sftp.ErrorCodes;

@XmlRootElement(name="testers.SftpRename")
public class SftpRename extends SftpOperation
{
	@XmlElement
	public String			oldpath = null;
	
	@XmlElement
	public String			newPath = null;
	
	@XmlElement
	public Integer			flags = null;
	
	@Override
	protected void runTest() throws Throwable
	{
		SshFxpInit			init = new SshFxpInit();
		SshFxpRename		rename = new SshFxpRename();
		ResponseStatus		response;
		
		Assert.assertNotNull(oldpath);
		Assert.assertNotNull(newPath);
		if (SftpConnection.protocolVersion >= 5)
			Assert.assertNotNull(flags);
		else
			Assert.assertNull("Field 'flags' is used only when using SFTP Protocol 5", flags);
		init.send(sftp.is, sftp.os, 1);
		
		response = rename.send(sftp.is, sftp.os, 2, oldpath, newPath, flags);
		if (response.status != ErrorCodes.SSH_FX_OK)
		{
			ResponseStatus	status = (ResponseStatus )response;
			
			Assert.fail("Can't do rename '" + oldpath + "'  : " + status.error);
		}
		response = rename.send(sftp.is, sftp.os, 2, newPath, oldpath, flags);
		if (response.status != ErrorCodes.SSH_FX_OK)
		{
			ResponseStatus	status = (ResponseStatus )response;
			
			Assert.fail("Can't do rename '" + oldpath + "'  : " + status.error);
		}
	}
	
	@Override
	public String toString()
	{
		return new ToStringBuilder(this).toString();
	}
}
