package mysecureshell.tests.testers;

import java.io.IOException;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import junit.framework.TestCase;
import mysecureshell.tests.utils.SftpConnection;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.junit.Assert;

@XmlRootElement(name="testers.SftpConfiguration")
public class SftpConfiguration extends TestCase
{
	@XmlElement
	public Integer					protocolVersion = 3;
	
	@XmlElement
	public String					configFile = null;
	
	@XmlElement
	public String					newConfig = null;
	
	@Override
	protected void runTest() throws Throwable
	{
		Assert.assertNotNull(configFile);
		Assert.assertNotNull(newConfig);
		SftpConnection.protocolVersion = protocolVersion;
		try
		{
			Path	file = FileSystems.getDefault().getPath(configFile);
			
			Assert.assertTrue("Config file '" + configFile + "' doesn't exists.", Files.exists(file));
			Files.write(file, newConfig.getBytes(), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
		}
		catch (IOException e)
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
