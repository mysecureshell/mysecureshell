package mysecureshell.tests.protocol;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.PacketSender;
import mysecureshell.tests.protocol.responses.ResponseStatus;
import mysecureshell.tests.utils.SftpConnection;

import org.junit.Assert;

import ch.ethz.ssh2.sftp.Packet;

public final class SshFxpRename
{	
	public ResponseStatus send(InputStream is, OutputStream os, Integer id, String oldPath, String newPath, Integer flags) throws IOException
	{
		PacketReceiver	receiver;
		PacketSender	sender = new PacketSender(Packet.SSH_FXP_RENAME);
		
		sender.writeUINT32(id);
		sender.writeString(oldPath);
		sender.writeString(newPath);
		if (SftpConnection.protocolVersion >= 5)
			sender.writeUINT32(flags);
		sender.write(os);
		receiver = new PacketReceiver().read(is);
		Assert.assertEquals(receiver.getPacketType(), Packet.SSH_FXP_STATUS);
		return new ResponseStatus(id).read(receiver);
	}
}
