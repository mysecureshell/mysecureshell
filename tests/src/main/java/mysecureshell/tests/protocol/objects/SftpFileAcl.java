package mysecureshell.tests.protocol.objects;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;
import org.apache.commons.lang3.builder.ToStringBuilder;

import ch.ethz.ssh2.packets.TypesReader;

public class SftpFileAcl
{
	public Integer	ace_type;
	public Integer	ace_flag;
	public Integer	ace_mask;
	public String	who;
	
	public static SftpFileAcl[] read(byte[] data) throws IOException
	{
		List<SftpFileAcl>	acls = new ArrayList<SftpFileAcl>();
		TypesReader			reader = new TypesReader(data);
		int					nbAcl, maxAcl;
		
		maxAcl = reader.readUINT32();
		for (nbAcl = 0; nbAcl < maxAcl; nbAcl++)
		{
			SftpFileAcl	acl = new SftpFileAcl();
			
			acl.ace_type = reader.readUINT32();
			acl.ace_flag = reader.readUINT32();
			acl.ace_mask = reader.readUINT32();
			acl.who = reader.readString();
			acls.add(acl);
		}
		return acls.toArray(new SftpFileAcl[acls.size()]);
	}


	public static void write(PacketSender sender, SftpFileAcl[] acls) throws IOException
	{
		sender.writeUINT32(acls.length);
		for (SftpFileAcl acl : acls)
		{
			sender.writeUINT32(acl.ace_type);
			sender.writeUINT32(acl.ace_flag);
			sender.writeUINT32(acl.ace_mask);
			sender.writeString(acl.who);
		}
	}

	@Override
	public int hashCode()
	{
		return new HashCodeBuilder()
			.append(ace_flag)
			.append(ace_mask)
			.append(ace_type)
			.append(who)
			.toHashCode();
	}


	@Override
	public boolean equals(Object obj)
	{
		if (obj == null) return false;
		if (obj == this) return true;
		if (obj.getClass() != getClass()) return false;

		SftpFileAcl o = (SftpFileAcl) obj;
		return new EqualsBuilder()
			.append(ace_flag, o.ace_flag)
			.append(ace_mask, o.ace_mask)
			.append(ace_type, o.ace_type)
			.append(who, o.who)
			.isEquals();
	}
	
	@Override
	public String toString()
	{
		return ToStringBuilder.reflectionToString(this);
	}
}
