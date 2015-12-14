package mysecureshell.tests.testers;

import java.io.IOException;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;

import junit.framework.TestCase;
import mysecureshell.tests.utils.SftpConnection;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.junit.Assert;

public class SftpConfiguration extends TestCase
{
	public Integer					protocolVersion = 3;
	public String					configFile = null;
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
