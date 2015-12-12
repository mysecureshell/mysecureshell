package mysecureshell.tests.testers.fs;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.Serializable;

import mysecureshell.tests.utils.RandomData;

import org.apache.commons.lang3.StringUtils;

public class FSFile implements Serializable
{
	public String				value;
	public transient char		type;
	public transient String		name;
	public transient int		rights;
	public transient String		owner, group;
	public transient String		additionalData;
	public transient String		aclRights;
	
	public FSFile()
	{
		this("f:file_or_directory");
	}
	
	public FSFile(String value)
	{
		String[]	parts = StringUtils.splitPreserveAllTokens(value, ':');
		
		this.rights = -1;
		for (int i = 0; i < parts.length; i++)
		{
			String	part = parts[i];

			if (part.length() == 0)
				continue;
			switch (i)
			{
			case 0: type = part.charAt(0); break;
			case 1: name = part; break;
			case 2: rights = Integer.parseInt(part); break;
			case 3: owner = part; break;
			case 4: group = part; break;
			case 5: additionalData = part.trim(); break;
			case 6: aclRights = part.trim().replace(',', ':'); break;
			default: break;
			}
		}
		this.value = value;
	}
	
	public boolean isFile()
	{
		return type == 'f';
	}
	
	public boolean isDirectory()
	{
		return type == 'd';
	}
	
	public boolean isLink()
	{
		return type == 'l';
	}
	
	public boolean hasContent()
	{
		return additionalData != null;
	}
	
	public boolean populateFile(File file)
	{
		OutputStream is;
		
		try
		{
			is = new FileOutputStream(file);
			
			is.write(getFileContent());
			is.close();
			return true;
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		return false;
	}
	
	private byte[] getFileContent()
	{
		RandomData	randomData;
		String[]	params;
		String		type;
		int			size;
		
		if (isFile() == false || additionalData.length() == 0)
			return null;
		params = StringUtils.splitPreserveAllTokens(additionalData, ',');
		type = params[0];
		if (params.length >= 2)
			size = Integer.parseInt(params[1]);
		else
			size = 1024;
		randomData = new RandomData(size);
		if (type.equalsIgnoreCase("random_binary"))
			return randomData.generateBinary();
		if (type.equalsIgnoreCase("random_string"))
			return randomData.generateString();
		throw new IllegalArgumentException("Unkow data generator: " + type);
	}
	
	@Override
	public String toString()
	{
		StringBuffer	sb = new StringBuffer();
		
		sb.append(type).append(':');
		sb.append(name);
		if (owner != null || group != null || rights != -1 || additionalData != null)
		{
			sb.append(':');
			if (rights != -1)
				sb.append(rights);
			if (owner != null || group != null || additionalData != null)
			{
				sb.append(':');
				if (owner != null)
					sb.append(owner);
				if (group != null || additionalData != null)
				{
					sb.append(':');
					if (group != null)
						sb.append(group);
					if (additionalData != null)
					{
						sb.append(':');
						sb.append(additionalData);
					}
				}
			}
		}
		return sb.toString();
	}
}
