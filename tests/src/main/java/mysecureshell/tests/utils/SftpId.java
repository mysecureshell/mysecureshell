package mysecureshell.tests.utils;

public final class SftpId
{
	private int	id;
	
	public SftpId()
	{
		id = 0;
	}
	
	public synchronized int getNextId()
	{
		return id++;
	}
}
