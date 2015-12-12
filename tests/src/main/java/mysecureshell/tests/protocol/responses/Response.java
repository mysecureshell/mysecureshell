package mysecureshell.tests.protocol.responses;

import java.io.IOException;

import mysecureshell.tests.protocol.objects.PacketReceiver;

import org.junit.Assert;


public abstract class Response
{
	private boolean	hasId;
	public int		id = 0;
	
	public Response()
	{
		hasId = false;
		id = -1;
	}
	
	public Response(int shouldId)
	{
		hasId = true;
		id = shouldId;
	}
	
	public Response read(PacketReceiver receiver) throws IOException
	{
		if (hasId)
		{
			int	newId = receiver.readUINT32();
			
			Assert.assertEquals("Trame is not excepted (" + id + " != " + newId + ")", id, newId);
		}
		return this;
	}
}
