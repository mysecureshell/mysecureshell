package mysecureshell.tests.testers.utils;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.Serializable;
import java.lang.reflect.Field;
import java.lang.reflect.Modifier;

public final class XMLWriter
{
	private BufferedWriter	writer;
	
	public XMLWriter(BufferedWriter writer) throws IOException
	{
		this.writer = writer;
		this.writer.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
		this.writer.newLine();
	}
	
	public boolean writeObject(Object obj) throws IOException
	{
		try
		{
			return writeObject(new StringBuffer(), obj, obj == null ? null : obj.getClass(), true);
		}
		catch (IllegalArgumentException e)
		{
			e.printStackTrace();
		}
		catch (IllegalAccessException e)
		{
			e.printStackTrace();
		}
		return false;
	}
	
	private boolean writeObject(StringBuffer padding, Object obj, Class<?> classObj, boolean showType) throws IOException, IllegalAccessException
	{
		boolean	isAutoClose = false;
		
		if (obj == null && classObj == null)
			return false;
		if (classObj.isArray())
			return writeObjectArray(padding, obj, classObj.getComponentType());
		if (showType)
		{
			writer.append(padding);
			writer.append('<');
			writer.append(classObj.getCanonicalName());
			if (obj == null)
				isAutoClose = true;
		}
		if (isSimpleSerializable(classObj, obj))
		{
			if (showType)
				writer.append('>');
			writer.write(obj.toString());
		}
		else if (obj != null)
		{
			writer.append('>');
			writer.newLine();
			padding.append(' ');
			for (Field field : classObj.getFields())
				if (Modifier.isPublic(field.getModifiers())
						&& Modifier.isTransient(field.getModifiers()) == false)
				{
					Class<?>	fieldClass = field.getType();
					Object		fieldValue = field.get(obj);

					if (fieldValue != null)
						fieldClass = fieldValue.getClass();
					writer.append(padding);
					writer.append('<');
					writer.append(field.getName());
					if (!fieldClass.getCanonicalName().startsWith("java.lang."))
					{
						writer.append(" type=\"");
						writer.append(fieldClass.getCanonicalName());
						writer.append('"');
					}
					if (fieldValue != null)
					{
						
						if (isSimpleSerializable(fieldClass, fieldValue) && fieldClass.isArray() == false)
						{
							writer.append('>');
							writeObject(padding, fieldValue, fieldClass, false);
						}
						else
						{
							if (fieldClass.isArray())
							{
								writer.append('>');
								writer.newLine();
							}
							writeObject(padding.append(' '), fieldValue, fieldClass, false);
							padding.deleteCharAt(padding.length() - 1);
							writer.append(padding);
						}
						writer.append("</");
						writer.append(field.getName());
						writer.append(">");
						writer.newLine();
					}
					else
					{
						writer.append("/>");
						writer.newLine();
					}
				}
			padding.deleteCharAt(padding.length() - 1);
		}
		if (showType)
		{
			if (isAutoClose)
				writer.append(" />");
			else
			{
				if (!isSimpleSerializable(classObj, obj))
					writer.append(padding);
				writer.append("</");
				writer.append(classObj.getCanonicalName());
				writer.append('>');
			}
			writer.newLine();
		}
		return true;
	}
	
	private boolean writeObjectArray(StringBuffer padding, Object obj, Class<?> classObj) throws IOException, IllegalAccessException
	{
		Object[]	array = (Object[] )obj;
		
		for (Object objInArray : array)
			writeObject(padding, objInArray, objInArray == null ? classObj : objInArray.getClass(), true);
		return true;
	}
	
	private boolean isSimpleSerializable(Class<?> typeObject, Object object)
	{
		return typeObject.isPrimitive() || object instanceof Serializable;
	}
	
	public void close() throws IOException
	{
		writer.flush();
		writer.close();
	}
}
