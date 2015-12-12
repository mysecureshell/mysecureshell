package mysecureshell.tests.protocol.objects;

import java.io.Serializable;
import java.util.Arrays;

import junit.framework.Assert;

import org.apache.commons.lang3.builder.HashCodeBuilder;

public class SftpHandle implements Serializable
{
	static private byte[]	_lastValue;
	
	private byte[]	handle;
	private boolean	isValid;
	
	public SftpHandle()
	{
		isValid = false;
	}
	
	public SftpHandle(byte[] handle)
	{
		this.handle = handle;
		_lastValue = handle;
		isValid = false;
	}
	
	public SftpHandle(String specialValue)
	{
		if (specialValue.equalsIgnoreCase("last_value"))
			handle = _lastValue;
		else if (specialValue.equalsIgnoreCase("valid"))
			isValid = true;
		else
			Assert.fail("Unkown handle special-value: " + specialValue);
	}
	
	public byte[] getBinaryHandle()
	{
		return handle;
	}

	@Override
	public int hashCode()
	{
		return new HashCodeBuilder()
			.toHashCode();
	}

	@Override
	public boolean equals(Object obj)
	{
		if (obj == null) return false;
		if (obj == this) return true;
		if (obj.getClass() != getClass()) return false;

		SftpHandle o = (SftpHandle) obj;
		if (isValid == false && o.isValid == false)
			if (!Arrays.equals(handle, o.handle)) return false;
		return true;
	}
	
	@Override
	public String toString()
	{
		StringBuffer	sb = new StringBuffer();
		
		if (isValid)
			sb.append("{VALID}");
		else
			sb.append(new String(handle));
			/*for (byte b : handle)
			{
				if (sb.length() > 0)
					sb.append(' ');
				sb.append(Integer.toHexString(b));
			}*/
		return sb.toString();
	}
}
