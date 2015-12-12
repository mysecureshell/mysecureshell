package mysecureshell.tests.protocol;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.PacketSender;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseExtFileHashing;
import mysecureshell.tests.protocol.responses.ResponseStatus;

import org.junit.Assert;

import ch.ethz.ssh2.sftp.Packet;

public final class SshFxpExtFileHashing
{	
	public Response hashFile(InputStream is, OutputStream os, Integer id, String fileName, String algorithm) throws IOException
	{
		return send(is, os, id, fileName, null, algorithm);
	}
	
	public Response hashHandle(InputStream is, OutputStream os, Integer id, byte[] handle, String algorithm) throws IOException
	{
		return send(is, os, id, null, handle, algorithm);
	}
	
	private Response send(InputStream is, OutputStream os, int id, String fileName, byte[] handle, String algorithm) throws IOException
	{
		PacketReceiver	receiver;
		PacketSender	sender = new PacketSender(Packet.SSH_FXP_EXTENDED);
		
		sender.writeUINT32(id);
		if (fileName != null)
		{
			sender.writeString("check-file-name");
			sender.writeString(fileName);
		}
		if (handle != null)
		{
			sender.writeString("check-file-handle");
			sender.writeUINT32(handle.length);
			sender.writeBytes(handle);
		}
		sender.writeString(algorithm);
		sender.writeUINT64(0);	//Offset
		sender.writeUINT64(0);	//Length (0: all file)
		sender.writeUINT32(0);	//BlocSize (0: one hash over the entire range)
		sender.write(os);
		receiver = new PacketReceiver().read(is);
		switch (receiver.getPacketType())
		{
		case Packet.SSH_FXP_EXTENDED_REPLY: return new ResponseExtFileHashing(id).read(receiver);
		case Packet.SSH_FXP_STATUS: return new ResponseStatus(id).read(receiver);
		default: Assert.fail("Unexpected packet-type: " + receiver.getPacketType());
		}
		return null;
	}
}
