package mysecureshell.tests;

import java.io.File;
import java.util.logging.Level;
import java.util.logging.Logger;

import junit.framework.TestCase;
import junit.framework.TestSuite;
import mysecureshell.tests.utils.FilesUtil;
import mysecureshell.tests.utils.XMLReader;


public class TestFiles
{
	private static Logger		log = Logger.getLogger("TestFiles");
	private static TestSuite	suite = null;
	
	public static junit.framework.Test suite() throws Exception
	{
		Logger.getLogger(Logger.GLOBAL_LOGGER_NAME).setLevel(Level.parse(System.getProperty("log", "ALL")));
		suite = new TestSuite("TestFiles");
		testAddTests();
		return suite;
	}
	
	public static void testAddTests() throws Exception
	{
		addTests(new File(".").listFiles());
	}
	
	public static void addTests(File[] tests) throws Exception
	{
		FilesUtil.sort(tests);
		for (File test : tests)
			if (test.isFile() && test.getName().startsWith("test") && test.getName().endsWith(".xml"))
				addTest(test);
	}
	
	public static void addTest(File test) throws Exception
	{
		XMLReader	xml;
		TestCase	testCase;
		String		testCaseName = test.getName();
		int			idx;
		
		if ((idx = testCaseName.lastIndexOf('.')) != -1)
			testCaseName = testCaseName.substring(0, idx);
		if (testCaseName.length() > 5)
			testCaseName = testCaseName.substring(5);
		log.info("Loading test: " + test);
		xml = new XMLReader(test);
		testCase = (TestCase )xml.readObject();
		testCase.setName(testCaseName);
		suite.addTest(testCase);
	}
}
