package mysecureshell.tests.utils;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.file.FileSystems;
import java.nio.file.Files;

import org.junit.Assert;

public class AssertUtils
{
	static public void assertSameFileBinary(String expectedFile, ByteArrayOutputStream actualBytes) throws IOException
	{
		assertSameFileBinary(Files.readAllBytes(FileSystems.getDefault().getPath(expectedFile)), actualBytes.toByteArray());
	}
	
	static public void assertSameFileBinary(ByteArrayOutputStream expectedBytes, String actualFile) throws IOException
	{
		assertSameFileBinary(expectedBytes.toByteArray(), Files.readAllBytes(FileSystems.getDefault().getPath(actualFile)));
	}
	
	static public void assertSameFileBinary(byte[] expectedBytes, byte[] actualBytes)
	{
		Assert.assertArrayEquals(expectedBytes, actualBytes);
	}
	
	static public void assertSameFileText(String expectedFile, ByteArrayOutputStream actualBytes) throws IOException
	{
		assertSameFileText(Files.readAllBytes(FileSystems.getDefault().getPath(expectedFile)), true, actualBytes.toByteArray(), false);
	}

	static public void assertSameFileText(ByteArrayOutputStream expectedBytes, String actualFile) throws IOException
	{
		assertSameFileText(expectedBytes.toByteArray(), true, Files.readAllBytes(FileSystems.getDefault().getPath(actualFile)), false);
	}
	
	static public void assertSameFileText(byte[] expectedBytes, boolean replaceCarriageReturnExpected, byte[] actualBytes, boolean replaceCarriageReturnActual)
	{
		String	expected, actual;

		actual = new String(actualBytes);
		if (replaceCarriageReturnActual)
			actual = actual.replaceAll("\r\n", "\n");
		expected = new String(expectedBytes);
		if (replaceCarriageReturnExpected)
			expected = expected.replaceAll("\r\n", "\n");
		Assert.assertEquals(expected, actual);
	}
}
