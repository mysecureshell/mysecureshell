package mysecureshell.tests.protocol.objects;

import java.io.IOException;
import java.io.InputStream;
import java.math.BigInteger;

import org.junit.Assert;

import ch.ethz.ssh2.packets.TypesReader;

public class PacketReceiver
{
	private static int MAX_SIZE = 65365;
	
	private TypesReader	reader = null;
	private int			packetType = -1;
	
	public PacketReceiver read(InputStream is) throws IOException
	{
		byte[]	msglen = new byte[4];
		byte[]	msg;
		int		len;

		readBytes(is, msglen, 0, 4);
		len = (((msglen[0] & 0xff) << 24) | ((msglen[1] & 0xff) << 16) | ((msglen[2] & 0xff) << 8) | (msglen[3] & 0xff));
		Assert.assertFalse("Illegal sftp packet len: " + len, (len > MAX_SIZE) || (len <= 0));
		msg = new byte[len];
		readBytes(is, msg, 0, len);
		reader = new TypesReader(msg);
		packetType = reader.readByte();
		return this;
	}
	
	private void readBytes(InputStream is, byte[] buff, int pos, int len) throws IOException
	{
		while (len > 0)
		{
			int	count = is.read(buff, pos, len);
			
			Assert.assertFalse("Unexpected end of sftp stream.", count < 0);
			Assert.assertFalse("Underlying stream implementation is bogus!", (count == 0) || (count > len));
			len -= count;
			pos += count;
		}
	}
	
	public int getPacketType()
	{
		return packetType;
	}
	
	public int readByte() throws IOException
	{
		return reader.readByte();
	}
	
	public byte[] readBytes(int len) throws IOException
	{
		return reader.readBytes(len);
	}
	
	public boolean readBoolean() throws IOException
	{
		return reader.readBoolean();
	}

	public int readUINT32() throws IOException
	{
		return reader.readUINT32();
	}

	public long readUINT64() throws IOException
	{
		return reader.readUINT64();
	}

	public BigInteger readMPINT() throws IOException
	{
		return reader.readMPINT();
	}

	public byte[] readByteString() throws IOException
	{
		return reader.readByteString();
	}

	public String readString(String charsetName) throws IOException
	{
		return reader.readString(charsetName);
	}

	public String readString() throws IOException
	{
		return reader.readString();
	}

	public String[] readNameList() throws IOException
	{
		return reader.readNameList();
	}

	public int remain()
	{
		return reader.remain();
	}
}
