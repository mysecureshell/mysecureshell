package mysecureshell.tests.testers;

import mysecureshell.tests.utils.Command;

import org.apache.commons.lang3.SystemUtils;
import org.junit.Assert;

public class SftpWho
{
	public static void userIsIdle()
	{
		System.out.println("User is idle ???");
		if (!SystemUtils.IS_OS_WINDOWS)
			Assert.assertEquals("idle", getUserStatus());
	}
	
	public static void userIsDownloading()
	{
		System.out.println("User is downloading ???");
		if (!SystemUtils.IS_OS_WINDOWS)
			Assert.assertEquals("download", getUserStatus());
	}
	
	public static void userIsUploading()
	{
		System.out.println("User is uploading ???");
		if (!SystemUtils.IS_OS_WINDOWS)
			Assert.assertEquals("upload", getUserStatus());
	}
	
	private static String getUserStatus()
	{
		String	user = System.getenv("serverUser");
		String	result;
		
		System.out.println("SftpWho FULL: [[" + Command.executeAndReturnResult("/bin/sh", "-c", "sftp-who") + "]]");
		result = Command.executeAndReturnResult("/bin/sh", "-c", "sftp-who | fgrep -A 2 'Name: " + user + "' | grep -Eo 'Status: (\\S*) ' | cut -d' ' -f2 | tr -d '\n'");
		System.out.println("SftpWho: [[" + result + "]]");
		return result;
	}
}
