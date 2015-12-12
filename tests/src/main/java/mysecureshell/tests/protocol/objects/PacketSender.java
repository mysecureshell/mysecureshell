package mysecureshell.tests.protocol.objects;

import java.io.IOException;
import java.io.OutputStream;

import ch.ethz.ssh2.packets.TypesWriter;

public class PacketSender extends TypesWriter
{
	public PacketSender(int packetType)
	{
		writeByte(packetType);
	}
	
	public void write(OutputStream os) throws IOException
	{
		int	msglen = length();

		os.write(msglen >> 24);
		os.write(msglen >> 16);
		os.write(msglen >> 8);
		os.write(msglen);
		os.write(getBytes());
	}
}
