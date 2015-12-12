package mysecureshell.tests.testers;

import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.security.MessageDigest;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import mysecureshell.tests.protocol.SshFxpExtFileHashing;
import mysecureshell.tests.protocol.SshFxpInit;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseExtFileHashing;
import mysecureshell.tests.protocol.responses.ResponseStatus;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.junit.Assert;

@XmlRootElement(name="testers.SftpExtFileHashing")
public class SftpExtFileHashing extends SftpOperation
{
	@XmlElement
	public String		file = null;
	
	@XmlElement
	public String		algorithm = null;
	
	@XmlElement
	public String		originalPath = null;
	
	@Override
	protected void runTest() throws Throwable
	{
		SshFxpInit				init = new SshFxpInit();
		SshFxpExtFileHashing	hash = new SshFxpExtFileHashing();
		ResponseExtFileHashing	fileHash;
		Response				response;
		
		Assert.assertNotNull(file);
		Assert.assertNotNull(algorithm);
		Assert.assertNotNull(originalPath);
		init.send(sftp.is, sftp.os, 1);
		response = hash.hashFile(sftp.is, sftp.os, 2, file, algorithm);
		if (response instanceof ResponseStatus)
		{
			ResponseStatus	status = (ResponseStatus )response;
			
			Assert.fail("Can't hash file '" + file + "' : " + status.error);
		}
		fileHash = (ResponseExtFileHashing) response;
		Assert.assertTrue(algorithm.equalsIgnoreCase(fileHash.algorithm));
		try
		{
			MessageDigest	messageDigest = MessageDigest.getInstance(algorithm);
			byte[]			originalHash;
			
			messageDigest.update(Files.readAllBytes(FileSystems.getDefault().getPath(originalPath)));
			originalHash = messageDigest.digest();
			Assert.assertArrayEquals(originalHash, fileHash.hash);
		}
		catch (Exception e)
		{
			Assert.fail(e.toString());
		}
	}
	
	@Override
	public String toString()
	{
		return new ToStringBuilder(this).toString();
	}
}
