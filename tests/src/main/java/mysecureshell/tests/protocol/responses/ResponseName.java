package mysecureshell.tests.protocol.responses;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import mysecureshell.tests.protocol.objects.PacketReceiver;
import mysecureshell.tests.protocol.objects.SftpFile;
import mysecureshell.tests.protocol.objects.SftpFileAttributes;
import mysecureshell.tests.utils.SftpConnection;

import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;

public final class ResponseName extends Response
{
	public SftpFile[]	names;
	
	public ResponseName()
	{
		super();
	}
	
	public ResponseName(int shouldId)
	{
		super(shouldId);
	}
	
	public ResponseName read(PacketReceiver receiver) throws IOException
	{
		List<SftpFile>	namesAsList;
		int				nbNames, maxNames;
		
		super.read(receiver);
		maxNames = receiver.readUINT32();
		namesAsList = new ArrayList<SftpFile>(maxNames);
		for (nbNames = 0; nbNames < maxNames; nbNames++)
		{
			SftpFile	pair;
			String		fileName;
			
			fileName = receiver.readString();
			if (SftpConnection.protocolVersion <= 3)
				receiver.readString();
			pair = new SftpFile(fileName, new SftpFileAttributes().read(receiver));
			namesAsList.add(pair);
		}
		names = namesAsList.toArray(new SftpFile[namesAsList.size()]);
		return this;
	}

	@Override
	public int hashCode()
	{
		return new HashCodeBuilder()
			.append(names)
			.toHashCode();
	}

	@Override
	public boolean equals(Object obj)
	{
		if (obj == null) return false;
		if (obj == this) return true;
		if (obj.getClass() != getClass()) return false;

		ResponseName o = (ResponseName) obj;
		return new EqualsBuilder()
			.append(names, o.names)
			.isEquals();
	}
}
