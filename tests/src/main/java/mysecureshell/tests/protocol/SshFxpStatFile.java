package mysecureshell.tests.protocol;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.PacketSender;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseAttrs;
import mysecureshell.tests.protocol.responses.ResponseStatus;
import mysecureshell.tests.utils.SftpConnection;

import org.junit.Assert;

import ch.ethz.ssh2.sftp.Packet;

public final class SshFxpStatFile
{	
	public Response sendFile(InputStream is, OutputStream os, Integer id, String fileName, Integer flags) throws IOException
	{
		return send(is, os, id, fileName, null, flags);
	}
	
	public Response sendHandle(InputStream is, OutputStream os, Integer id, byte[] handle, Integer flags) throws IOException
	{
		return send(is, os, id, null, handle, flags);
	}
	
	private Response send(InputStream is, OutputStream os, int id, String fileName, byte[] handle, int flags) throws IOException
	{
		PacketReceiver	receiver;
		PacketSender	sender = new PacketSender(Packet.SSH_FXP_STAT);
		
		sender.writeUINT32(id);
		if (fileName != null)
			sender.writeString(fileName);
		if (handle != null)
		{
			sender.writeUINT32(handle.length);
			sender.writeBytes(handle);
		}
		if (SftpConnection.protocolVersion >= 4)
			sender.writeUINT32(flags);
		sender.write(os);
		receiver = new PacketReceiver().read(is);
		switch (receiver.getPacketType())
		{
		case Packet.SSH_FXP_ATTRS: return new ResponseAttrs(id).read(receiver);
		case Packet.SSH_FXP_STATUS: return new ResponseStatus(id).read(receiver);
		default: Assert.fail("Unexpected packet-type: " + receiver.getPacketType());
		}
		return null;
	}
}
