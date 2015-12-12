package mysecureshell.tests;

import java.io.File;
import java.util.logging.Level;
import java.util.logging.Logger;

import mysecureshell.tests.utils.FilesUtil;
import junit.framework.TestCase;
import junit.framework.TestSuite;

public class TestValgrindResults
{
	private static TestSuite	suite	= null;

	public static junit.framework.Test suite() throws Exception
	{
		Logger.getLogger(Logger.GLOBAL_LOGGER_NAME).setLevel(Level.parse(System.getProperty("log", "ALL")));
		suite = new TestSuite("TestFiles");
		testAddResults();
		return suite;
	}

	public static void testAddResults() throws Exception
	{
		addResults(new File("/tmp").listFiles());
	}

	public static void addResults(File[] results) throws Exception
	{
		FilesUtil.sort(results);
		for (File result : results)
			if (result.isFile() && result.getName().startsWith("MSS.log."))
				addResult(result);
	}

	public static void addResult(File result) throws Exception
	{
		TestCase testCase;
		String testCaseName = result.getName();

		testCase = new TestValgrind(result);
		testCase.setName(testCaseName);
		suite.addTest(testCase);
	}
}
