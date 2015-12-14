package mysecureshell.tests.testers.utils;

import java.io.File;
import java.io.IOException;
import java.io.Serializable;
import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

public final class XMLReader
{
	private Document	doc;
	
	public XMLReader(String file) throws ParserConfigurationException, SAXException, IOException
	{
		this(new File(file));
	}
	
	public XMLReader(File file) throws ParserConfigurationException, SAXException, IOException
	{
		DocumentBuilderFactory	builderFactory = DocumentBuilderFactory.newInstance();
		DocumentBuilder			builder = builderFactory.newDocumentBuilder();
		
		doc = builder.parse(file);
	}
	
	public Object readObject() throws IOException
	{
		try
		{
			return readObject(null, doc.getDocumentElement(), false);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		return null;
	}
	
	private Object readObject(String objType, Node node, boolean complexType) throws ClassNotFoundException, InstantiationException, IllegalAccessException, NoSuchFieldException
	{
		String		className = objType == null ? node.getNodeName() : objType;
		Class<?>	classObj;
		Object		obj = null;
		
		if (node == null)
			return null;
		if (className.startsWith("testers.") || className.startsWith("protocol."))
			className = "mysecureshell.tests." + className;
		if (className.endsWith("[]"))
		{
			List<Object>	array = new ArrayList<Object>();
			
			classObj = Class.forName(className.substring(0, className.length() - 2));
			while (node != null)
			{	
				if (node.getNodeType() != Node.TEXT_NODE)
				{
					array.add(readObject(node.getNodeName(), node, false));
				}
				node = node.getNextSibling();
			}
			obj = Array.newInstance(classObj, array.size());
			for (int i = 0; i < array.size(); i++)
				Array.set(obj, i, array.get(i));
			return obj;
		}
		classObj = Class.forName(className);
		if (isSimpleSerializable(classObj))
		{
			try
			{
				Constructor<?>	constructor = classObj.getConstructor(String.class);

				obj = constructor.newInstance(node.getFirstChild() == null ? node.getNodeValue() : node.getFirstChild().getNodeValue());
			}
			catch (NoSuchMethodException e)
			{
				e.printStackTrace();
			}
			catch (IllegalArgumentException e)
			{
				e.printStackTrace();
			}
			catch (InvocationTargetException e)
			{
				e.printStackTrace();
			}
		}
		else
		{
			NodeList	childs = node.getChildNodes();
			int			nbNodes, maxNodes;
			
			if (complexType)
				childs = node.getParentNode().getChildNodes();
			maxNodes = childs.getLength();
			obj = classObj.newInstance();
			for (nbNodes = 0; nbNodes < maxNodes; nbNodes++)
			{
				String	propertyName;
				String	propertyType;
				Field	property;
				Node	params = childs.item(nbNodes);
				Node	paramsChild = params.getFirstChild();
				
				if (params.getNodeType() == Node.TEXT_NODE || params.getNodeType() == Node.COMMENT_NODE)
					continue;
				propertyName = params.getNodeName();
				property = classObj.getField(propertyName);
				if (params.getAttributes().getNamedItem("type") != null)
					propertyType = params.getAttributes().getNamedItem("type").getNodeValue();
				else
					propertyType = property.getType().getCanonicalName();
				if (paramsChild != null)
					while (paramsChild.getNodeType() == Node.TEXT_NODE && paramsChild.getNextSibling() != null)
						paramsChild = paramsChild.getNextSibling();
				property.set(obj, readObject(propertyType, paramsChild, true));
			}
		}
		return obj;
	}
	
	private boolean isSimpleSerializable(Class<?> typeObject)
	{
		return typeObject.isPrimitive() || Serializable.class.isAssignableFrom(typeObject);
	}
}
