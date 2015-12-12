package mysecureshell.tests.utils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.logging.Logger;

import org.junit.Assert;

import ch.ethz.ssh2.Connection;
import ch.ethz.ssh2.Session;
import ch.ethz.ssh2.sftp.Packet;

public class SftpConnection
{
	public static int protocolVersion = 3;
	
	private static Logger	log = Logger.getLogger("SftpConnection");
	private Connection	connection = null;
	private Session		session = null;
	public InputStream	is = null;
	public InputStream	isErr = null;
	public OutputStream	os = null;
	
	public SftpConnection(String host, String user, String pass) throws IOException
	{
		log.info("Connecting to " + host + " ...");
		connection = new Connection(host);
		connection.connect();
		Assert.assertTrue("Error during authentification", connection.authenticateWithPassword(user, pass));
		session = connection.openSession();
		Assert.assertNotNull(session);
		session.startSubSystem("sftp");
		is = session.getStdout();
		Assert.assertNotNull(is);
		isErr = session.getStderr();
		Assert.assertNotNull(isErr);
		os = session.getStdin();
		Assert.assertNotNull(os);
	}
	
	public void closeConnection() throws IOException
	{
		if (is != null)
		{
			Assert.assertEquals("Has " + is.available() +" bytes availabe", is.available(), 0);
			is = null;
		}
		if (isErr != null)
		{
			Assert.assertEquals("Has " + isErr.available() +" bytes availabe", isErr.available(), 0);
			isErr = null;
		}
		if (session != null)
		{
			log.info("Closing session...");
			session.close();
			session = null;
		}
		if (connection != null)
		{
			log.info("Closing connection...");
			connection.close();
			connection = null;
		}
	}
	
	private void sendMessage(int type, int requestId, byte[] msg, int off, int len) throws IOException
	{
		int	msglen = len + 1;

		if (type != Packet.SSH_FXP_INIT)
			msglen += 4;
		os.write(msglen >> 24);
		os.write(msglen >> 16);
		os.write(msglen >> 8);
		os.write(msglen);
		os.write(type);
		if (type != Packet.SSH_FXP_INIT)
		{
			os.write(requestId >> 24);
			os.write(requestId >> 16);
			os.write(requestId >> 8);
			os.write(requestId);
		}
		os.write(msg, off, len);
		os.flush();
	}
	
	public void sendMessage(int type, int requestId, byte[] msg) throws IOException
	{
		sendMessage(type, requestId, msg, 0, msg.length);
	}

	private void readBytes(byte[] buff, int pos, int len) throws IOException
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

	public byte[] receiveMessage() throws IOException
	{
		return receiveMessage(34000);
	}
	
	private byte[] receiveMessage(int maxlen) throws IOException
	{
		byte[]	msglen = new byte[4];
		byte[]	msg;
		int		len;

		readBytes(msglen, 0, 4);
		len = (((msglen[0] & 0xff) << 24) | ((msglen[1] & 0xff) << 16) | ((msglen[2] & 0xff) << 8) | (msglen[3] & 0xff));
		Assert.assertFalse("Illegal sftp packet len: " + len, (len > maxlen) || (len <= 0));
		msg = new byte[len];
		readBytes(msg, 0, len);
		return msg;
	}
}
