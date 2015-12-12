package mysecureshell.tests.protocol;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.PacketSender;
import mysecureshell.tests.protocol.responses.ResponseInit;

import org.junit.Assert;

import ch.ethz.ssh2.sftp.Packet;

public final class SshFxpExtended
{	
	private PacketSender send(Integer id, String extented) throws IOException
	{
		PacketSender	sender = new PacketSender(Packet.SSH_FXP_EXTENDED);
		
		sender.writeUINT32(id);
		sender.writeString(extented);
		return sender;
	}
	
	public ResponseInit dummy(InputStream is, OutputStream os, Integer id, String dummy) throws IOException
	{
		PacketReceiver	receiver;
		PacketSender	sender = send(id, "dummy-extension");
		
		sender.writeString(dummy);
		sender.write(os);
		receiver = new PacketReceiver().read(is);
		Assert.assertEquals(receiver.getPacketType(), Packet.SSH_FXP_EXTENDED_REPLY);
		return new ResponseInit().read(receiver);
	}
}
