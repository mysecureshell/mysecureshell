package mysecureshell.tests.protocol;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.PacketSender;
import mysecureshell.tests.protocol.objects.SftpFileAttributes;
import mysecureshell.tests.protocol.responses.Response;
import mysecureshell.tests.protocol.responses.ResponseHandle;
import mysecureshell.tests.protocol.responses.ResponseStatus;
import mysecureshell.tests.utils.SftpConnection;

import org.junit.Assert;

import ch.ethz.ssh2.sftp.Packet;

public final class SshFxpOpenFile
{	
	public Response send(InputStream is, OutputStream os, Integer id, String fileName, Integer desiredAccess, Integer pflags, SftpFileAttributes attrs) throws IOException
	{
		PacketReceiver	receiver;
		PacketSender	sender = new PacketSender(Packet.SSH_FXP_OPEN);
		
		sender.writeUINT32(id);
		sender.writeString(fileName);
		if (SftpConnection.protocolVersion >= 5)
			sender.writeUINT32(desiredAccess);
		sender.writeUINT32(pflags);
		attrs.write(sender);
		sender.write(os);
		receiver = new PacketReceiver().read(is);
		switch (receiver.getPacketType())
		{
		case Packet.SSH_FXP_HANDLE: return new ResponseHandle(id).read(receiver);
		case Packet.SSH_FXP_STATUS: return new ResponseStatus(id).read(receiver);
		default: Assert.fail("Unexpected packet-type: " + receiver.getPacketType());
		}
		return null;
	}
}
