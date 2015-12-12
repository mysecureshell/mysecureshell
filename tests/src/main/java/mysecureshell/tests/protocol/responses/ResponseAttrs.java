package mysecureshell.tests.protocol.responses;

import java.io.IOException;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.SftpFileAttributes;

import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;

public final class ResponseAttrs extends Response
{
	public SftpFileAttributes	attributes;
	
	public ResponseAttrs()
	{
		super();
	}
	
	public ResponseAttrs(int shouldId)
	{
		super(shouldId);
	}
	
	public ResponseAttrs read(PacketReceiver receiver) throws IOException
	{
		super.read(receiver);
		attributes = new SftpFileAttributes().read(receiver);
		return this;
	}

	@Override
	public int hashCode()
	{
		return new HashCodeBuilder()
			.append(attributes)
			.toHashCode();
	}

	@Override
	public boolean equals(Object obj)
	{
		if (obj == null) return false;
		if (obj == this) return true;
		if (obj.getClass() != getClass()) return false;

		ResponseAttrs o = (ResponseAttrs) obj;
		return new EqualsBuilder()
			.append(attributes, o.attributes)
			.isEquals();
	}
}
