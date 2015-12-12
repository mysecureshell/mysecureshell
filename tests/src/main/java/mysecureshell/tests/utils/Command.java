package mysecureshell.tests.utils;

import java.io.File;
import java.io.IOException;

public final class Command
{
	private Command()
	{
	}
	
	public static boolean execute(String... cmd)
	{
		return execute(null, cmd);
	}
	
	public static boolean execute(File parentPath, String... cmd)
	{
		boolean	returnValue = false;
		
		try
		{
			Process	p = Runtime.getRuntime().exec(cmd, null, parentPath);
			
			try
			{
				if (p.waitFor() == 0)
					returnValue = true;
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();
			}
			p.destroy();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		return returnValue;
	}
	
	public static String executeAndReturnResult(String... cmd)
	{
		return executeAndReturnResult(null, cmd);
	}
	
	public static String executeAndReturnResult(File parentPath, String... cmd)
	{
		String	result = null;
		
		try
		{
			Process	p = Runtime.getRuntime().exec(cmd, null, parentPath);
			
			try
			{
				if (p.waitFor() == 0)
				{
					byte[]	data = new byte[p.getInputStream().available()];
					
					p.getInputStream().read(data);
					result = new String(data);
				}
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();
			}
			p.destroy();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		return result;
	}
}
