package mysecureshell.tests.testers;

import java.io.BufferedWriter;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.StringWriter;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import junit.framework.TestCase;
import mysecureshell.tests.utils.SftpConnection;
import mysecureshell.tests.utils.XMLWriter;

import org.apache.commons.lang3.builder.EqualsBuilder;
import org.junit.Assert;
import org.junit.Assume;

@XmlRootElement(name="testers.SftpFunction")
public class SftpFunction extends TestCase
{
	private transient static Logger	log = Logger.getLogger("SftpFunction");
	public transient SftpConnection	sftp = null;
	
	@XmlElement
	public Object[]	params = null;
	
	@XmlElement
	public String	className = null;
	
	@XmlElement
	public String	methodName = "send";
	
	@XmlElement
	public Object	resultExcepted = null;
	
	public void runTest() throws Exception
	{
		Class<?>[]	paramsType = null;
		Method		method = null;
		Object		result = null;
		
		Assume.assumeNotNull(sftp, className, methodName, params);
		if (params != null)
		{
			paramsType = new Class[params.length + 3];
			paramsType[0] = InputStream.class;
			paramsType[1] = OutputStream.class;
			paramsType[2] = Integer.class;
			for (int i = 0; i < params.length; i++)
				if (params[i] != null)
					paramsType[i + 3] = params[i].getClass();
		}
		try
		{
			Object[]	paramsPrepared = new Object[params.length + 3];
			Class<?>	classObj = Class.forName("protocol." + className);
			
			paramsPrepared[0] = sftp.is;
			paramsPrepared[1] = sftp.os;
			paramsPrepared[2] = 42;
			System.arraycopy(params, 0, paramsPrepared, 3, params.length);
			method = classObj.getMethod(methodName, paramsType);
			log.info("Should invoke: '" + method + "' with params:" + Arrays.asList(params));
			result = method.invoke(classObj.newInstance(), paramsPrepared);
			log.finer("Result: " + result);
			log.finer("Expect result: " + resultExcepted);
			if (log.isLoggable(Level.FINEST))
			{
				StringWriter	sw = new StringWriter();
				XMLWriter		xml = new XMLWriter(new BufferedWriter(sw));
				
				xml.writeObject(result);
				xml.close();
				log.finest("Serialized result: " + sw);
			}
			Assert.assertTrue("Not excepted result '" + result + "' != '" + resultExcepted + "'",
					new EqualsBuilder().append(result, resultExcepted).isEquals());
		}
		catch (Exception e)
		{
			e.printStackTrace();
			Assert.fail(e.toString());
		}
	}
	
	@Override
	public String getName()
	{
		return methodName == null ? super.getName() : methodName;
	}
}
