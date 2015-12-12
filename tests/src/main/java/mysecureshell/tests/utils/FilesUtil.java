package mysecureshell.tests.utils;

import java.io.File;
import java.util.Arrays;
import java.util.Comparator;

import mysecureshell.tests.protocol.objects.SftpFile;

public class FilesUtil
{
	public static void sort(File[] files)
	{
		Arrays.sort(files, new Comparator<File>()
		{
			public int compare(File o1, File o2)
			{
				return o1.compareTo(o2);
			}
		});
	}

	public static void sort(SftpFile[] files)
	{
		Arrays.sort(files, new Comparator<SftpFile>()
		{
			public int compare(SftpFile o1, SftpFile o2)
			{
				return o1.file.compareTo(o2.file);
			}
		});
	}
}
