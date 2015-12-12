package mysecureshell.tests.protocol.objects;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import mysecureshell.tests.utils.SftpConnection;

import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;
import org.apache.commons.lang3.builder.ToStringBuilder;

import ch.ethz.ssh2.sftp.AttribFlags;

public class SftpFileAttributes
{
	public Integer				attrib = 0;
	public Integer				flags = 0;
	public Integer				type = 0;
	public Long					size = 0L;
	public String				owner = null;
	public String				group = null;
	public Integer				permissions = 0;
	public Long					atime = 0L;
	public Long					createtime = 0L;
	public Long					mtime = 0L;
	public SftpFileAcl[]		acl = null;
	public SftpFileExtend[]		extended = null;
	
	public SftpFileAttributes read(PacketReceiver packet) throws IOException
	{
		flags = packet.readUINT32();
		if (SftpConnection.protocolVersion >= 4)
			type = packet.readByte();
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_SIZE) == AttribFlags.SSH_FILEXFER_ATTR_SIZE)
			size = packet.readUINT64();
		if (SftpConnection.protocolVersion <= 3 && (flags & AttribFlags.SSH_FILEXFER_ATTR_V3_UIDGID) == AttribFlags.SSH_FILEXFER_ATTR_V3_UIDGID)
		{
			owner = Integer.toString(packet.readUINT32());
			group = Integer.toString(packet.readUINT32());
		}
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_OWNERGROUP) == AttribFlags.SSH_FILEXFER_ATTR_OWNERGROUP)
		{
			owner = packet.readString();
			group = packet.readString();
		}
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_PERMISSIONS) == AttribFlags.SSH_FILEXFER_ATTR_PERMISSIONS)
			permissions = packet.readUINT32();
		if (SftpConnection.protocolVersion <= 3)
		{
			if  ((flags & AttribFlags.SSH_FILEXFER_ATTR_V3_ACMODTIME) == AttribFlags.SSH_FILEXFER_ATTR_V3_ACMODTIME)
			{
				atime = (long) packet.readUINT32();
				mtime = (long) packet.readUINT32();
			}
		}
		else //SftpConnection.protocolVersion >= 4
		{
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_ACCESSTIME) == AttribFlags.SSH_FILEXFER_ATTR_ACCESSTIME)
				atime = packet.readUINT64();
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_SUBSECOND_TIMES) == AttribFlags.SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
				packet.readUINT32();
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_CREATETIME) == AttribFlags.SSH_FILEXFER_ATTR_CREATETIME)
				createtime = packet.readUINT64();
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_SUBSECOND_TIMES) == AttribFlags.SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
				packet.readUINT32();
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_MODIFYTIME) == AttribFlags.SSH_FILEXFER_ATTR_MODIFYTIME)
				mtime = packet.readUINT64();
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_SUBSECOND_TIMES) == AttribFlags.SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
				packet.readUINT32();
		}
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_ACL) == AttribFlags.SSH_FILEXFER_ATTR_ACL)
			acl = SftpFileAcl.read(packet.readByteString());
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_BITS) == AttribFlags.SSH_FILEXFER_ATTR_BITS)
			attrib = packet.readUINT32();
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_EXTENDED) == AttribFlags.SSH_FILEXFER_ATTR_EXTENDED)
		{
			List<SftpFileExtend>	listExtends = new ArrayList<SftpFileExtend>();
			
			if (SftpConnection.protocolVersion < 5)
			{
				while (packet.remain() > 0)
				{
					String	extType = packet.readString();
					byte[]	extData = packet.readByteString();
					
					listExtends.add(new SftpFileExtend(extType, extData));
				}
			}
			else // 'maxExtended' only defined when SFTP protocol >= 5
			{
				int	nbExtended, maxExtended = packet.readUINT32();
				
				for (nbExtended = 0; nbExtended < maxExtended; nbExtended++)
				{
					String	extType = packet.readString();
					byte[]	extData = packet.readByteString();
					
					listExtends.add(new SftpFileExtend(extType, extData));
				}
			}
			extended = listExtends.toArray(new SftpFileExtend[listExtends.size()]);
		}
		return this;
	}

	public void write(PacketSender sender) throws IOException
	{
		sender.writeUINT32(flags);
		if (SftpConnection.protocolVersion >= 4)
			sender.writeByte(type);
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_SIZE) == AttribFlags.SSH_FILEXFER_ATTR_SIZE)
			sender.writeUINT64(size);
		if (SftpConnection.protocolVersion <= 3 && (flags & AttribFlags.SSH_FILEXFER_ATTR_V3_UIDGID) == AttribFlags.SSH_FILEXFER_ATTR_V3_UIDGID)
		{
			sender.writeUINT32(Integer.parseInt(owner));
			sender.writeUINT32(Integer.parseInt(group));
		}
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_OWNERGROUP) == AttribFlags.SSH_FILEXFER_ATTR_OWNERGROUP)
		{
			sender.writeString(owner);
			sender.writeString(group);
		}
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_PERMISSIONS) == AttribFlags.SSH_FILEXFER_ATTR_PERMISSIONS)
			sender.writeUINT32(permissions);
		
		
		if (SftpConnection.protocolVersion <= 3)
		{
			if  ((flags & AttribFlags.SSH_FILEXFER_ATTR_V3_ACMODTIME) == AttribFlags.SSH_FILEXFER_ATTR_V3_ACMODTIME)
			{
				sender.writeUINT32(atime.intValue());
				sender.writeUINT32(mtime.intValue());
			}
		}
		else //SftpConnection.protocolVersion >= 5
		{		
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_ACCESSTIME) == AttribFlags.SSH_FILEXFER_ATTR_ACCESSTIME)
				sender.writeUINT64(atime);
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_SUBSECOND_TIMES) == AttribFlags.SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
				sender.writeUINT32(0);
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_CREATETIME) == AttribFlags.SSH_FILEXFER_ATTR_CREATETIME)
				sender.writeUINT64(createtime);
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_SUBSECOND_TIMES) == AttribFlags.SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
				sender.writeUINT32(0);
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_MODIFYTIME) == AttribFlags.SSH_FILEXFER_ATTR_MODIFYTIME)
				sender.writeUINT64(mtime);
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_SUBSECOND_TIMES) == AttribFlags.SSH_FILEXFER_ATTR_SUBSECOND_TIMES)
				sender.writeUINT32(0);
		}
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_ACL) == AttribFlags.SSH_FILEXFER_ATTR_ACL)
			SftpFileAcl.write(sender, acl);
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_BITS) == AttribFlags.SSH_FILEXFER_ATTR_BITS)
			sender.writeUINT32(attrib);
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_EXTENDED) == AttribFlags.SSH_FILEXFER_ATTR_EXTENDED)
		{
      if (SftpConnection.protocolVersion >= 5)
  			sender.writeUINT32(extended.length);
			for (SftpFileExtend pair : extended)
			{
				sender.writeString(pair.extension);
				sender.writeBytes(pair.data);
			}
		}
	}

	@Override
	public int hashCode()
	{
		return new HashCodeBuilder()
			.append(attrib)
			.append(flags)
			.append(type)
			.append(size)
			.append(owner)
			.append(group)
			.append(permissions)
			.append(atime)
			.append(createtime)
			.append(mtime)
			.append(acl)
			.append(extended)
			.toHashCode();
	}

	@Override
	public boolean equals(Object obj)
	{
		if (obj == null) return false;
		if (obj == this) return true;
		if (obj.getClass() != getClass()) return false;

		SftpFileAttributes	o = (SftpFileAttributes) obj;
		EqualsBuilder		eb = new EqualsBuilder();
		
		if (SftpConnection.protocolVersion >= 4)
			eb.append((int) type, (int)o.type);
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_SIZE) == AttribFlags.SSH_FILEXFER_ATTR_SIZE)
			eb.append((long )size, (long )o.size);
		if (SftpConnection.protocolVersion <= 3 && (flags & AttribFlags.SSH_FILEXFER_ATTR_V3_UIDGID) == AttribFlags.SSH_FILEXFER_ATTR_V3_UIDGID)
		{
			eb.append(owner, o.owner);
			eb.append(group, o.group);
		}
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_OWNERGROUP) == AttribFlags.SSH_FILEXFER_ATTR_OWNERGROUP)
		{
			eb.append(owner, o.owner);
			eb.append(group, o.group);
		}
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_PERMISSIONS) == AttribFlags.SSH_FILEXFER_ATTR_PERMISSIONS)
			eb.append((int) permissions, (int) o.permissions);
		if (SftpConnection.protocolVersion <= 3)
		{
			if  ((flags & AttribFlags.SSH_FILEXFER_ATTR_V3_ACMODTIME) == AttribFlags.SSH_FILEXFER_ATTR_V3_ACMODTIME)
			{
				eb.append((long) atime, (long) o.atime);
				eb.append((long) mtime, (long) o.mtime);
			}
		}
		else
		{		
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_ACCESSTIME) == AttribFlags.SSH_FILEXFER_ATTR_ACCESSTIME)
				eb.append((long) atime, (long) o.atime);
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_CREATETIME) == AttribFlags.SSH_FILEXFER_ATTR_CREATETIME)
				eb.append((long) createtime, (long) o.createtime);
			if ((flags & AttribFlags.SSH_FILEXFER_ATTR_MODIFYTIME) == AttribFlags.SSH_FILEXFER_ATTR_MODIFYTIME)
				eb.append((long) mtime, (long) o.mtime);
		}
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_ACL) == AttribFlags.SSH_FILEXFER_ATTR_ACL)
			eb.append(acl, o.acl);
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_BITS) == AttribFlags.SSH_FILEXFER_ATTR_BITS)
			eb.append((int) attrib, (int) o.attrib);
		if ((flags & AttribFlags.SSH_FILEXFER_ATTR_EXTENDED) == AttribFlags.SSH_FILEXFER_ATTR_EXTENDED)
			eb.append(extended, o.extended);
		return eb.isEquals();
	}
	
	@Override
	public String toString()
	{
		return ToStringBuilder.reflectionToString(this);
	}
}
