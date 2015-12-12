package mysecureshell.tests.protocol.responses;

import java.io.IOException;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.SftpHandle;

import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;

public final class ResponseHandle extends Response
{
	public SftpHandle	handle;
	
	public ResponseHandle()
	{
		super();
	}
	
	public ResponseHandle(int shouldId)
	{
		super(shouldId);
	}
	
	public byte[] getBinaryHandle()
	{
		return handle.getBinaryHandle();
	}
	
	public ResponseHandle read(PacketReceiver receiver) throws IOException
	{
		super.read(receiver);
		handle = new SftpHandle(receiver.readByteString());
		return this;
	}

	@Override
	public int hashCode()
	{
		return new HashCodeBuilder()
			.append(handle)
			.toHashCode();
	}

	@Override
	public boolean equals(Object obj)
	{
		if (obj == null) return false;
		if (obj == this) return true;
		if (obj.getClass() != getClass()) return false;

		ResponseHandle o = (ResponseHandle) obj;
		return new EqualsBuilder()
			.append(handle, o.handle)
			.isEquals();
	}
	
	@Override
	public String toString()
	{
		return "ResponseHandle {handle: " + handle + "}";
	}
}
