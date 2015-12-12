package mysecureshell.tests.testers;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.util.logging.Logger;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import junit.framework.TestCase;
import mysecureshell.tests.utils.SftpConnection;
import mysecureshell.tests.utils.XMLWriter;

import org.junit.Assert;

@XmlRootElement(name="testers.SftpOperation")
public class SftpOperation extends TestCase
{
	private static Logger	log = Logger.getLogger("SftpOperation");
	transient protected SftpConnection	sftp = null;
	transient protected boolean			doDumpResult = false;
	
	@XmlElement
	public Boolean						mustFail = Boolean.FALSE;
	
	@Override
	protected void setUp() throws Exception
	{
		String	host, user, password, dump;

		log.info("Playing " + this.getName());
		host = System.getenv("serverHost");
		Assert.assertNotNull("Check if 'serverHost' is set in env", host);
		user = System.getenv("serverUser");
		Assert.assertNotNull("Check if 'serverUser' is set in env", user);
		password = System.getenv("serverPassword");
		Assert.assertNotNull("Check if 'serverPassword' is set in env", password);
		dump = System.getenv("dump");
		if (dump != null)
			doDumpResult = Boolean.parseBoolean(dump);
		sftp = new SftpConnection(host, user, password);
	}
	
	@Override
	protected void tearDown() throws Exception
	{
		String	errTxt = null;
		
		while (sftp.isErr.available() > 0)
		{
			if (errTxt == null)
				errTxt = String.valueOf((char) sftp.isErr.read());
			else
				errTxt += (char) sftp.isErr.read();
		}
		
		sftp.closeConnection();
		if (doDumpResult)
		{
			XMLWriter 	xml;
			String		fileName = "test_" + this.getName() + ".dump";
			
			xml = new XMLWriter(new BufferedWriter(new FileWriter(new File(fileName))));
			xml.writeObject(this);
			xml.close();
		}
		Assert.assertNull(errTxt, errTxt);
	}
}
