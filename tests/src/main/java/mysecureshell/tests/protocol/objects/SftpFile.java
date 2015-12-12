package mysecureshell.tests.protocol.objects;

import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;
import org.apache.commons.lang3.builder.ToStringBuilder;

public class SftpFile
{
	public String 				file;
	public SftpFileAttributes	attributes;
	
	public SftpFile()
	{
		;
	}
	
	public SftpFile(String file, SftpFileAttributes attributes)
	{
		this.file = file;
		this.attributes = attributes;
	}
	
	@Override
	public int hashCode()
	{
		return new HashCodeBuilder()
			.append(file)
			.append(attributes)
			.toHashCode();
	}

	@Override
	public boolean equals(Object obj)
	{
		if (obj == null) return false;
		if (obj == this) return true;
		if (obj.getClass() != getClass()) return false;

		SftpFile o = (SftpFile) obj;
		return new EqualsBuilder()
			.append(file, o.file)
			.append(attributes, o.attributes)
			.isEquals();
	}
	
	@Override
	public String toString()
	{
		return new ToStringBuilder(this)
			.append("file", file)
			.append("attributes", attributes)
			.toString();
	}
}
