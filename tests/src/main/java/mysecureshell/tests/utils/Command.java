package mysecureshell.tests.utils;

import java.io.File;
import java.io.IOException;
import java.util.logging.Logger;

public final class Command
{
	private static Logger	log = Logger.getLogger("Command");
	
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
				String	stdOut, stdErr;
				byte[]	data;
				
				if (p.waitFor() == 0)
					returnValue = true;
				data = new byte[p.getInputStream().available()];
				p.getInputStream().read(data);
				stdOut = new String(data);
				data = new byte[p.getErrorStream().available()];
				p.getErrorStream().read(data);
				stdErr = new String(data);
				log.info("Call of '" + cmd[0] + "':\nSTDOUT=" + stdOut + "\nSTDERR=" + stdErr);
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
