package mysecureshell.tests.protocol;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.PacketSender;
import mysecureshell.tests.protocol.responses.ResponseInit;
import mysecureshell.tests.utils.SftpConnection;

import org.junit.Assert;

import ch.ethz.ssh2.sftp.Packet;

public final class SshFxpInit
{
	public ResponseInit send(InputStream is, OutputStream os, Integer id) throws IOException
	{
		PacketReceiver	receiver;
		PacketSender	sender = new PacketSender(Packet.SSH_FXP_INIT);
		
		sender.writeUINT32(SftpConnection.protocolVersion);
		sender.write(os);
		receiver = new PacketReceiver().read(is);
		Assert.assertEquals(receiver.getPacketType(), Packet.SSH_FXP_VERSION);
		return new ResponseInit().read(receiver);
	}
}
