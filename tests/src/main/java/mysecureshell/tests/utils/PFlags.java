package mysecureshell.tests.utils;

public class PFlags
{
	/**
	 * Open the file for reading.
	 */
	public static int SSH2_FXF_READ		= 0x00000001;
	
	/**
	 * Open the file for writing.  If both this and SSH_FXF_READ are
	 * specified, the file is opened for both reading and writing.
	 */
	public static int SSH2_FXF_WRITE		= 0x00000002;
	
	/**
	 * Force all writes to append data at the end of the file.  The
	 * offset parameter to write will be ignored.
	 */
	public static int SSH2_FXF_APPEND	= 0x00000004;
	
	/**
	 * If this flag is specified, then a new file will be created if one
	 * does not already exist (if O_TRUNC is specified, the new file will
	 * be truncated to zero length if it previously exists).
	 */
	public static int SSH2_FXF_CREAT		= 0x00000008;
	
	/**
	 * Forces an existing file with the same name to be truncated to zero
	 * length when creating a file by specifying SSH_FXF_CREAT.
	 * SSH_FXF_CREAT MUST also be specified if this flag is used.
	 */
	public static int SSH2_FXF_TRUNC		= 0x00000010;
	
	/**
	 * Causes the request to fail if the named file already exists.
	 * SSH_FXF_CREAT MUST also be specified if this flag is used.
	 */
	public static int SSH2_FXF_EXCL		= 0x00000020;
	
	/**
	 * Indicates that the server should treat the file as text and
	 * convert it to the canonical newline convention in use.  (See
	 * Determining Server Newline Convention. (Section 4.3)
	 */
	public static int SSH4_FXF_TEXT		= 0x00000040;
	
	
	public static int SSH5_FXF_CREATE_NEW					= 0x00000000;
	public static int SSH5_FXF_CREATE_TRUNCATE				= 0x00000001;
	public static int SSH5_FXF_OPEN_EXISTING				= 0x00000002;
	public static int SSH5_FXF_OPEN_OR_CREATE				= 0x00000003;
	public static int SSH5_FXF_TRUNCATE_EXISTING			= 0x00000004;
	public static int SSH5_FXF_ACCESS_DISPOSITION			= 0x00000007;
	public static int SSH5_FXF__FLAGS						= 0x0000007F;
	public static int SSH5_FXF_ACCESS_APPEND_DATA			= 0x00000008;
	public static int SSH5_FXF_ACCESS_APPEND_DATA_ATOMIC	= 0x00000010;
	public static int SSH5_FXF_ACCESS_TEXT_MODE				= 0x00000020;
	public static int SSH5_FXF_ACCESS_READ_LOCK				= 0x00000040;
	public static int SSH5_FXF_ACCESS_WRITE_LOCK			= 0x00000080;
	public static int SSH5_FXF_ACCESS_DELETE_LOCK			= 0x00000100;
	public static int SSH5_FXF_ACCESS__FLAGS				= 0x000001F8;
}
