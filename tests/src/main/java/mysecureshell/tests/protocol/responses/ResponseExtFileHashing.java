package mysecureshell.tests.protocol.responses;

import java.io.IOException;

import mysecureshell.tests.protocol.objects.PacketReceiver;

import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;

public final class ResponseExtFileHashing extends Response
{
	public String	algorithm;
	public byte[]	hash;
	
	public ResponseExtFileHashing()
	{
		super();
	}
	
	public ResponseExtFileHashing(int shouldId)
	{
		super(shouldId);
	}
	
	public ResponseExtFileHashing read(PacketReceiver receiver) throws IOException
	{
		super.read(receiver);
		algorithm = receiver.readString();
		hash = receiver.readBytes(receiver.remain());
		return this;
	}

	@Override
	public int hashCode()
	{
		return new HashCodeBuilder()
			.append(algorithm)
			.append(hash)
			.toHashCode();
	}

	@Override
	public boolean equals(Object obj)
	{
		if (obj == null) return false;
		if (obj == this) return true;
		if (obj.getClass() != getClass()) return false;

		ResponseExtFileHashing o = (ResponseExtFileHashing) obj;
		return new EqualsBuilder()
			.append(algorithm, o.algorithm)
			.append(hash, o.hash)
			.isEquals();
	}
}
