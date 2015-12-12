package mysecureshell.tests.protocol;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.PacketSender;
import mysecureshell.tests.protocol.objects.SftpFileAttributes;
import mysecureshell.tests.protocol.responses.ResponseStatus;

import org.junit.Assert;

import ch.ethz.ssh2.sftp.Packet;

public final class SshFxpMakeDir
{	
	public ResponseStatus send(InputStream is, OutputStream os, Integer id, String path, SftpFileAttributes attrs) throws IOException
	{
		PacketReceiver	receiver;
		PacketSender	sender = new PacketSender(Packet.SSH_FXP_MKDIR);
		
		sender.writeUINT32(id);
		sender.writeString(path);
		attrs.write(sender);
		sender.write(os);
		receiver = new PacketReceiver().read(is);
		Assert.assertEquals(Packet.SSH_FXP_STATUS, receiver.getPacketType());
		return new ResponseStatus(id).read(receiver);
	}
}
