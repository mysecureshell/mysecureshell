package mysecureshell.tests.protocol;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.PacketSender;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseData;
import mysecureshell.tests.protocol.responses.ResponseStatus;

import org.junit.Assert;

import ch.ethz.ssh2.sftp.Packet;

public final class SshFxpReadFile
{	
	public Response send(InputStream is, OutputStream os, Integer id, byte[] handle, Long offset, Integer len) throws IOException
	{
		PacketReceiver	receiver;
		PacketSender	sender = new PacketSender(Packet.SSH_FXP_READ);
		
		sender.writeUINT32(id);
		sender.writeUINT32(handle.length);
		sender.writeBytes(handle);
		sender.writeUINT64(offset);
		sender.writeUINT32(len);
		sender.write(os);
		receiver = new PacketReceiver().read(is);
		switch (receiver.getPacketType())
		{
		case Packet.SSH_FXP_DATA: return new ResponseData(id).read(receiver);
		case Packet.SSH_FXP_STATUS: return new ResponseStatus(id).read(receiver);
		default: Assert.fail("Unexpected packet-type: " + receiver.getPacketType());
		}
		return null;
	}
}
