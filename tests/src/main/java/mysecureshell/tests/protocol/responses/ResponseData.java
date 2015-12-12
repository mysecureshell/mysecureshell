package mysecureshell.tests.protocol.responses;

import java.io.IOException;

import mysecureshell.tests.protocol.objects.PacketReceiver;

import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;

public final class ResponseData extends Response
{
	public byte[]	data;
	
	public ResponseData()
	{
		super();
	}
	
	public ResponseData(int shouldId)
	{
		super(shouldId);
	}
	
	public int getDataLength()
	{
		return data.length;
	}
	
	public ResponseData read(PacketReceiver receiver) throws IOException
	{
		super.read(receiver);
		data = receiver.readByteString();
		return this;
	}

	@Override
	public int hashCode()
	{
		return new HashCodeBuilder()
			.append(data)
			.toHashCode();
	}

	@Override
	public boolean equals(Object obj)
	{
		if (obj == null) return false;
		if (obj == this) return true;
		if (obj.getClass() != getClass()) return false;

		ResponseData o = (ResponseData) obj;
		return new EqualsBuilder()
			.append(data, o.data)
			.isEquals();
	}
}
