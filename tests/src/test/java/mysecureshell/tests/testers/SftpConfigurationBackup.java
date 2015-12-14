package mysecureshell.tests.testers;

import java.io.IOException;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;

import junit.framework.TestCase;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.junit.Assert;

public class SftpConfigurationBackup extends TestCase
{
	public String					oldConfig = null;
	public String					newConfig = null;
	public Boolean					deleteOldConfig = false;
	
	@Override
	protected void runTest() throws Throwable
	{
		Assert.assertNotNull(oldConfig);
		Assert.assertNotNull(newConfig);
		
		try
		{
			Path	srcFile = FileSystems.getDefault().getPath(oldConfig);
			Path	dstFile = FileSystems.getDefault().getPath(newConfig);
			
			Assert.assertTrue("Source file '" + oldConfig + "' doesn't exists.", Files.exists(srcFile));
			Files.deleteIfExists(dstFile);			
			Files.copy(srcFile, dstFile, LinkOption.NOFOLLOW_LINKS);
			if (deleteOldConfig)
				Files.delete(srcFile);
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
