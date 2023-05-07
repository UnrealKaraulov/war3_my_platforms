<?php

namespace Warcis\ServerNickName\FtpConnectionEngine;

class SUPERFTP
{
	//ftp://username:password@sld.domain.tld/
	public static function getFtpConnection($ip,$port,$username,$password,$dir) 
	{ 
		// Set up a connection 
		$conn = ftp_connect($ip,$port,15); 
		
		if ($conn)
		{
			// Login 
			if (ftp_login($conn, $username, $password)) 
			{ 
				ftp_pasv($conn, true);

				// Change the dir 
				if (strlen($dir)> 0)
				{
					ftp_chdir($conn, $dir); 
				}
				else 
				{
					ftp_chdir($conn, "."); 
				}
				
				// Return the resource 
				return $conn; 
			} 
		}
		// Or retun null 
		return null; 
	} 
	
	public static function getFtpFile($conn_id, $local_file, $file)
	{
		if (ftp_get($conn_id, $local_file, $file, FTP_BINARY)) 
		{
			return true;
		} 
		else 
		{
			return false;
		}
	}
	
	public static function setFtpFile($conn_id, $remote_file, $file)
	{
		if (ftp_put($conn_id, $remote_file, $file, FTP_BINARY)) 
		{
			return true;
		} 
		else 
		{
			return false;
		}
	}
	
	public static function delFtpFile($conn_id, $remote_file)
	{
		if (ftp_delete($conn_id, $remote_file)) 
		{
			return true;
		} 
		else 
		{
			return false;
		}
	}
	
	public static function renFtpFile($conn_id, $oldfile,$newfile)
	{
		if (ftp_rename ($conn_id, $oldfile, $newfile)) 
		{
			return true;
		} 
		else 
		{
			return false;
		}
	}
	
	public static function getFtpFileContents($conn_id , $file)
	{
		ob_start();
		$result = ftp_get($conn_id, "php://output", $file, FTP_BINARY);
		$data = ob_get_contents();
		ob_end_clean();
		if ($result)
			return $data;
		return null;
	}
	
	public static function getFtpFileContentsWithSize($conn_id , $file)
	{
		ob_start();
		$result = ftp_get($conn_id, "php://output", $file, FTP_BINARY);
		$data = ob_get_contents();
		$datasize = ob_get_length( );
		ob_end_clean();
		if ($result)
			return array( 'data' => $data, 'size' => $datasize );
		return null;
	}
	
	public static function getFileSize($conn_id , $file)
	{
		return ftp_size($conn_id, $file);
	}
	
	public static function ifFileExistsAndNoZero($conn_id , $file)
	{
		return ftp_size($conn_id, $file) > 0;
	}
	
	
	
	public static function getDownloadFileToBrowser($conn_id , $file)
	{
		$size = ftp_size($conn_id, $file);
		header("Content-Type: application/octet-stream");
		header("Content-Disposition: attachment; filename= $file");
		header("Content-Length: $size"); 
		$result = ftp_get($conn_id, "php://output", $file, FTP_BINARY);
		ftp_close($conn_id);
		exit( );
	}
	
	public static function getFtpFileContentsForDownload($conn_id , $file)
	{
		$result = ftp_get($conn_id, "php://output", $file, FTP_BINARY);
		if ($result)
			return $data;
		return null;
	}
	
	
}