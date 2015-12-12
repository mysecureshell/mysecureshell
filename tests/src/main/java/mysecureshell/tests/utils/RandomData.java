package mysecureshell.tests.utils;

import java.io.UnsupportedEncodingException;
import java.util.Random;

import org.apache.commons.lang3.RandomStringUtils;

public class RandomData
{
	private int	size;
	
	public RandomData(int size)
	{
		this.size = size;
	}
	
	public byte[] generateBinary()
	{
		byte[]	data = new byte[size];
		
		new Random().nextBytes(data);
		return data;
	}

	public byte[] generateString()
	{
		String[]	toInsert = { "\r", "\n", "\r\n", "\r\r", "\n\n", "\r\n\r\n" };
		int			pos = 0;
		
		for (String insertMe : toInsert)
			pos += insertMe.length();
		try
		{
			StringBuffer	sb;
			int				dec;

			if (pos > size)
				return RandomStringUtils.randomAscii(size).getBytes("UTF-8");
			sb = new StringBuffer(RandomStringUtils.randomAscii(size - pos));
			dec = sb.length() / toInsert.length;
			pos = 0;
			for (String insertMe : toInsert)
			{
				pos += dec;
				sb.insert(pos, insertMe);
			}
			return sb.toString().getBytes("UTF-8");
		}
		catch (UnsupportedEncodingException e)
		{
			e.printStackTrace();
		}
		return null;
	}
}
