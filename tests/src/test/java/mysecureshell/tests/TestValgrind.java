package mysecureshell.tests;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.apache.commons.lang3.builder.ToStringBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class TestValgrind extends TestCase
{
	private File	file;
	
	public TestValgrind(File xmlFile)
	{
		this.file = xmlFile;
	}
	
	@Override
	protected void runTest() throws Throwable
	{
		DocumentBuilderFactory	builderFactory = DocumentBuilderFactory.newInstance();
		DocumentBuilder			builder = builderFactory.newDocumentBuilder();
		Document				doc = builder.parse(file);
		
		parseError(doc.getElementsByTagName("error"));
	}
	
	private void parseError(NodeList nodes)
	{
		for (int i = 0; i < nodes.getLength(); i++)
			parseError(nodes.item(i));
	}

	private void parseError(Node item)
	{
		NodeList		childs = item.getChildNodes();
		List<String>	stack = null;
		String			kind = null;
		String			what = null;
		String			xwhat = null;
		
		for (int i = 0; i < childs.getLength(); i++)
		{
			Node	child = childs.item(i);
			
			if ("kind".equals(child.getNodeName()))
				kind = child.getTextContent();
			else if ("xwhat".equals(child.getNodeName()))
				xwhat = parseWhat(child);
			else if ("stack".equals(child.getNodeName()))
				stack = parseStack(child);
			else if ("what".equals(child.getNodeName()))
				what = child.getTextContent();
		}
		if (kind.equalsIgnoreCase("Leak_DefinitelyLost"))
		{
			String		secondStack = stack.get(1);
			boolean		ignoreMe = secondStack.startsWith("/build/buildd/eglibc-2.13/nss/nsswitch.c");
			
			if (!ignoreMe)
				ignoreMe = secondStack.startsWith("/build/buildd/eglibc-2.15/nss/nsswitch.c");
			if (!ignoreMe)
				Assert.fail(xwhat + " # Leak in " + stack);
		}
		else if (kind.equalsIgnoreCase("InvalidFree"))
		{
			Assert.fail(what + " # " + stack);
		}
		else if (kind.equalsIgnoreCase("InvalidWrite"))
		{
			Assert.fail(what + " # " + stack);
		}
		else
			Assert.fail("Unkown kind: " + kind);
	}
	
	private String parseWhat(Node item)
	{
		NodeList	childs = item.getChildNodes();
		
		for (int i = 0; i < childs.getLength(); i++)
		{
			Node	child = childs.item(i);
			
			if ("text".equals(child.getNodeName()))
				return child.getTextContent();
		}
		return item.getTextContent();
	}
	
	private List<String> parseStack(Node item)
	{
		NodeList		childs = item.getChildNodes();
		List<String>	stack = new ArrayList<String>();
		
		for (int i = 0; i < childs.getLength(); i++)
		{
			Node	child = childs.item(i);
			
			if ("frame".equals(child.getNodeName()))
				stack.add(parseFrame(child));
		}
		return stack;
	}
	
	private String parseFrame(Node item)
	{
		NodeList	childs = item.getChildNodes();
		String		ip = null;
		String		dir = null;
		String		file = null;
		String		line = null;
		String		obj = null;
		String		func = null;
		
		for (int i = 0; i < childs.getLength(); i++)
		{
			Node	child = childs.item(i);
			
			if ("dir".equals(child.getNodeName()))
				dir = child.getTextContent();
			else if ("file".equals(child.getNodeName()))
				file = child.getTextContent();
			else if ("fn".equals(child.getNodeName()))
				func = child.getTextContent();
			else if ("ip".equals(child.getNodeName()))
				ip = child.getTextContent();
			else if ("line".equals(child.getNodeName()))
				line = child.getTextContent();
			else if ("obj".equals(child.getNodeName()))
				obj = child.getTextContent();
		}
		if (dir != null && file != null && line != null)
			return dir + "/" + file + " at line " + line;
		if (obj != null && func != null)
			return obj + " in function " + func;
		return "at memory " + ip;
	}

	@Override
	public String toString()
	{
		return new ToStringBuilder(this).toString();
	}
}
