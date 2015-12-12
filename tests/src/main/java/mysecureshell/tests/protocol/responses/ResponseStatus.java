package mysecureshell.tests.protocol.responses;

import java.io.IOException;

import mysecureshell.tests.protocol.objects.PacketReceiver;

import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;

public final class ResponseStatus extends Response
{
	public Integer	status;
	public String	error;
	public String	language;
	
	public ResponseStatus()
	{
		super();
	}
	
	public ResponseStatus(int shouldId)
	{
		super(shouldId);
	}
	
	public ResponseStatus read(PacketReceiver receiver) throws IOException
	{
		super.read(receiver);
		status = receiver.readUINT32();
		error = receiver.readString();
		language = receiver.readString();
		return this;
	}

	@Override
	public int hashCode()
	{
		return new HashCodeBuilder()
			.append(error)
			.append(language)
			.append(status)
			.toHashCode();
	}

	@Override
	public boolean equals(Object obj)
	{
		if (obj == null) return false;
		if (obj == this) return true;
		if (obj.getClass() != getClass()) return false;

		ResponseStatus o = (ResponseStatus) obj;
		return new EqualsBuilder()
			.append(error, o.error)
			.append(language, o.language)
			.append((int) status, (int) o.status)
			.isEquals();
	}
	
	@Override
	public String toString()
	{
		return "ResponseStatus {status: " + status + " language: " + language + " error: " + error + "}";
	}
}
