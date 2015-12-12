package mysecureshell.tests.testers;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import junit.framework.TestCase;
import mysecureshell.tests.utils.SftpConnection;

import org.junit.Assert;
import org.junit.Assume;

@XmlRootElement(name="testers.SftpSession")
public class SftpSession extends TestCase
{
	@XmlElement
	public SftpFunction[]				steps;
	
	transient private SftpConnection	sftp;
	
	@Override
	protected void setUp() throws Exception
	{
		String	host, user, password;

		host = System.getenv("serverHost");
		Assert.assertNotNull("Check if 'serverHost' is set in env", host);
		user = System.getenv("serverUser");
		Assert.assertNotNull("Check if 'serverUser' is set in env", user);
		password = System.getenv("serverPassword");
		Assert.assertNotNull("Check if 'serverPassword' is set in env", password);
		sftp = new SftpConnection(host, user, password);
	}
	
	@Override
	protected void runTest() throws Throwable
	{
		Assume.assumeNotNull((Object )steps);
		for (SftpFunction step : steps)
		{
			step.sftp = sftp;
			step.runTest();
		}
	}
	
	@Override
	protected void tearDown() throws Exception
	{
		sftp.closeConnection();
	}
}
