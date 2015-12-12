package mysecureshell.tests.protocol;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.PacketSender;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseName;
import mysecureshell.tests.protocol.responses.ResponseStatus;

import org.junit.Assert;

import ch.ethz.ssh2.sftp.Packet;

public final class SshFxpRealPath
{	
	public Response send(InputStream is, OutputStream os, Integer id, String pathName) throws IOException
	{
		PacketReceiver	receiver;
		PacketSender	sender = new PacketSender(Packet.SSH_FXP_REALPATH);
		
		sender.writeUINT32(id);
		sender.writeString(pathName);
		sender.write(os);
		receiver = new PacketReceiver().read(is);
		switch (receiver.getPacketType())
		{
		case Packet.SSH_FXP_NAME: return new ResponseName(id).read(receiver);
		case Packet.SSH_FXP_STATUS: return new ResponseStatus(id).read(receiver);
		default: Assert.fail("Unexpected packet-type: " + receiver.getPacketType());
		}
		return null;
	}
}
