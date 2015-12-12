package mysecureshell.tests.utils;

import java.io.File;
import java.io.IOException;

import javax.xml.bind.JAXBContext;
import javax.xml.parsers.ParserConfigurationException;

import mysecureshell.tests.testers.LocalFileSystem;
import mysecureshell.tests.testers.SftpConfiguration;
import mysecureshell.tests.testers.SftpConfigurationBackup;
import mysecureshell.tests.testers.SftpCreateAndRemoveDir;
import mysecureshell.tests.testers.SftpCreateAndRemoveSymLink;
import mysecureshell.tests.testers.SftpDownloadFile;
import mysecureshell.tests.testers.SftpExtFileHashing;
import mysecureshell.tests.testers.SftpFunction;
import mysecureshell.tests.testers.SftpListDir;
import mysecureshell.tests.testers.SftpOperation;
import mysecureshell.tests.testers.SftpRealPath;
import mysecureshell.tests.testers.SftpRename;
import mysecureshell.tests.testers.SftpSession;
import mysecureshell.tests.testers.SftpStatFile;
import mysecureshell.tests.testers.SftpUploadFile;

import org.xml.sax.SAXException;

public final class XMLReader
{
	private File	file;
	
	public XMLReader(String file) throws ParserConfigurationException, SAXException, IOException
	{
		this(new File(file));
	}
	
	public XMLReader(File file) throws ParserConfigurationException, SAXException, IOException
	{
		this.file = file;
	}
	
	public Object readObject() throws IOException
	{
		try
		{
			JAXBContext jaxbContext = JAXBContext.newInstance(LocalFileSystem.class, SftpConfiguration.class, SftpConfigurationBackup.class, SftpCreateAndRemoveDir.class, SftpCreateAndRemoveSymLink.class, SftpDownloadFile.class, SftpExtFileHashing.class, SftpFunction.class, SftpListDir.class, SftpOperation.class, SftpRealPath.class, SftpRename.class, SftpSession.class, SftpStatFile.class, SftpUploadFile.class);
			
			return jaxbContext.createUnmarshaller().unmarshal(file);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		return null;
	}
}
