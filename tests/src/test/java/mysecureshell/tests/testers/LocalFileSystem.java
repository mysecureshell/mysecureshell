package mysecureshell.tests.testers;

import java.io.File;
import java.io.IOException;
import java.nio.file.FileSystems;
import java.nio.file.FileVisitResult;
import java.nio.file.FileVisitor;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.logging.Logger;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import junit.framework.Assert;
import junit.framework.TestCase;
import mysecureshell.tests.testers.fs.FSFile;
import mysecureshell.tests.utils.Command;

import org.apache.commons.lang3.SystemUtils;
import org.junit.Assume;

@XmlRootElement(name="testers.LocalFileSystem")
public class LocalFileSystem extends TestCase
{
	private transient static Logger	log = Logger.getLogger("LocalFileSystem");
	
	@XmlElement
	public String	deleteBefore;
	
	@XmlElement
	public FSFile[]	content;
	
	@Override
	protected void runTest() throws Throwable
	{
		Assume.assumeNotNull((Object )content);
		Assert.assertEquals("You must be 'root' !", "root", System.getProperty("user.name"));
		runDeleteContent();
		runCreateContent();
	}
	
	private void runDeleteContent() throws Throwable
	{
		if (deleteBefore != null)
		{
			Path	dir = FileSystems.getDefault().getPath(deleteBefore);
			
			Files.walkFileTree(dir, new FileVisitor<Path>()
					{
						public FileVisitResult postVisitDirectory(Path dir, IOException exc) throws IOException
						{
							Files.delete(dir);
							return FileVisitResult.CONTINUE;
						}
		
						public FileVisitResult preVisitDirectory(Path dir, BasicFileAttributes attrs) throws IOException
						{
							return FileVisitResult.CONTINUE;
						}
		
						public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException
						{
							Files.delete(file);
							return FileVisitResult.CONTINUE;
						}
		
						public FileVisitResult visitFileFailed(Path file, IOException exc) throws IOException
						{
							log.throwing("LocalFileSystem", "runDeleteContent", exc);
							return FileVisitResult.CONTINUE;
					}});
		}
	}
	
	private void runCreateContent() throws Throwable
	{
		for (FSFile file : content)
		{
			log.info("Create '" + file.name + "' of type '" + file.type + "'");
			if (file.isDirectory())
			{
				Assert.assertTrue("Cannot create directory : " + file.name, new File(file.name).mkdirs());
				if (SystemUtils.IS_OS_UNIX)
				{
					if (file.rights != -1)
						Assert.assertTrue("Cannot change rights of directory : " + file.name + " => " + file.rights,
								Command.execute("chmod", String.valueOf(file.rights), file.name));
					if (file.owner != null)
						Assert.assertTrue("Cannot change owner of directory : " + file.name + " => " + file.owner,
								Command.execute("chown", file.owner, file.name));
					if (file.group != null)
						Assert.assertTrue("Cannot change group of directory : " + file.name + " => " + file.group,
								Command.execute("chgrp", file.group, file.name));
					if (file.aclRights != null)
						Assert.assertTrue("Cannot change acl of directory : " + file.name + " => " + file.aclRights,
								Command.execute("setfacl", "-m", file.aclRights, file.name));
				}
			}
			else if (file.isFile())
			{
				File	f = new File(file.name);
				
				Assert.assertTrue("Cannot create file : " + file.name, f.createNewFile());
				if (file.hasContent())
					Assert.assertTrue("Cannot populate file : " + file.name, file.populateFile(f));
				if (SystemUtils.IS_OS_UNIX)
				{
					if (file.rights != -1)
						Assert.assertTrue("Cannot change rights of file : " + file.name + " => " + file.rights,
								Command.execute("chmod", String.valueOf(file.rights), file.name));
					if (file.owner != null)
						Assert.assertTrue("Cannot change owner of file : " + file.name + " => " + file.owner,
								Command.execute("chown", file.owner, file.name));
					if (file.group != null)
						Assert.assertTrue("Cannot change group of file : " + file.name + " => " + file.group,
								Command.execute("chgrp", file.group, file.name));
					if (file.aclRights != null)
						Assert.assertTrue("Cannot change acl of file : " + file.name + " => " + file.aclRights,
								Command.execute("setfacl", "-m", file.aclRights, file.name));
				}
			}
			else if (file.isLink())
			{
				if (SystemUtils.IS_OS_UNIX)
				{
					Assert.assertTrue("Cannot make link : " + file.name + " => " + file.additionalData,
					Command.execute("ln", "-s", String.valueOf(file.rights), file.additionalData, file.name));
					if (file.owner != null)
						Assert.assertTrue("Cannot change owner of link : " + file.name + " => " + file.owner,
								Command.execute("chown", "-h", file.owner, file.name));
					if (file.group != null)
						Assert.assertTrue("Cannot change group of link : " + file.name + " => " + file.group,
								Command.execute("chgrp", "-h", file.group, file.name));
				}
				else
					Assert.fail("No link for your system !");
			}
			else
			{
				Assert.fail("Unkown type: " + file.type);
			}
		}
	}
}
