<?php 
require 'IniParser/IniParser.php';
//where the files are

//has a filepath name been passed?
if(!empty($_GET['mappath']) && strlen($_GET['mappath']) > 3){
	//protect from people getting other files
	$filepath = "./maplistfullnew/".urldecode($_GET['mappath']);
	$filename = urldecode($_GET['mappath']);
	//does the filepath exist?
	if(file_exists($filepath) && file_exists($filepath.'.ini')){

	
		$MapConfigParser = new IniParser($filepath.'.ini');
		$MapConfig = $MapConfigParser->parse();
		$MapDownloadsCount = intval($MapConfig->PrintMapListInfo->Downloads);
		$MapConfig->PrintMapListInfo->Downloads = $MapDownloadsCount + 1;
		$MapConfigParser->SaveChanges( $MapConfig);
		

		//set force download headers
		header('Content-Description: File Transfer');
		header('Content-Type: application/octet-stream');
		header('Content-Disposition: attachment; filename="'.$filename.'"');
		header('Content-Transfer-Encoding: binary');
		header('Connection: Keep-Alive');
		header('Expires: 0');
		header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
		header('Pragma: public');
		header('Content-Length: ' . sprintf("%u", filesize($filepath)));

		//open and output filepath contents
		$fh = fopen($filepath, "rb");
		while (!feof($fh)) {
			echo fgets($fh);
			flush();
		}
		fclose($fh);
		exit;
	}else{
		//header("HTTP/1.0 404 Not Found");
		exit('File not found!');
	}
}else{
	exit(header("Location: ./index.html"));
}
?>