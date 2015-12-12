package mysecureshell.tests.protocol.responses;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.SftpProtocolExtend;
import mysecureshell.tests.utils.SftpConnection;

import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;
import org.junit.Assert;

public final class ResponseInit extends Response
{
	public int					version;
	public boolean				ignoreExtensions;
	public SftpProtocolExtend[]	extensions;
	public String				newLine;
	public String				supported;
	
	public ResponseInit()
	{
		super();
		ignoreExtensions = false;
	}
	
	public ResponseInit read(PacketReceiver receiver) throws IOException
	{
		List<SftpProtocolExtend>	extensionsAsArray = new ArrayList<SftpProtocolExtend>();
		
		super.read(receiver);
		version = receiver.readUINT32();
		while (receiver.remain() > 0)
		{
			SftpProtocolExtend	ext;
			String				extName = receiver.readString();
			String				extValue = receiver.readString();
			
			if (SftpConnection.protocolVersion >= 4 && extName.equals("newline"))
				newLine = extValue;
			else if (SftpConnection.protocolVersion >= 5 && extName.equals("supported"))
				supported = extValue;
			ext = new SftpProtocolExtend(extName, extValue);
			Assert.assertFalse("Extension already declared '" + extName + "'", extensionsAsArray.contains(ext));
			extensionsAsArray.add(ext);
		}
		extensions = extensionsAsArray.toArray(new SftpProtocolExtend[extensionsAsArray.size()]);
		if (SftpConnection.protocolVersion >= 4)
			Assert.assertNotNull(newLine);
		if (SftpConnection.protocolVersion >= 5)
			Assert.assertNotNull(supported);
		return this;
	}

	@Override
	public int hashCode()
	{
		return new HashCodeBuilder()
			.append(version)
			.toHashCode();
	}

	@Override
	public boolean equals(Object obj)
	{
		if (obj == null) return false;
		if (obj == this) return true;
		if (obj.getClass() != getClass()) return false;

		ResponseInit o = (ResponseInit) obj;
		EqualsBuilder eb = new EqualsBuilder().append(version, o.version);
		
		if (ignoreExtensions == false && o.ignoreExtensions == false)
			eb.append(extensions, o.extensions);
		return eb.isEquals();
	}
	
	@Override
	public String toString()
	{
		return "ResponseInit {version:" + version + " ignoreExtensions:" + ignoreExtensions + " extensions:" + extensions + "}";
	}
}
