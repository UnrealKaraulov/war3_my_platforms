<?php

namespace Warcis\ServerNickName\Pub\Controller;

use XF\Mvc\ParameterBag;
use XF\Mvc\FormAction;
use XF\Mvc\Reply\View;
use XF\Pub\Controller\AbstractController;

use Warcis\ServerNickName\PvPGNBot;
use Warcis\ServerNickName\MapUploadEngine\WarcisMapDB;
use Warcis\ServerNickName\FtpConnectionEngine\SUPERFTP;
use Warcis\ServerNickName\WarcraftStatsEngine\WarcraftStats;
use Warcis\ServerNickName\Pub\Controller\MapIconExtractor;

require_once "ExternalScripts".DIRECTORY_SEPARATOR."TriggerHappyMPQreader".DIRECTORY_SEPARATOR."mpq.php";
require_once "ExternalScripts".DIRECTORY_SEPARATOR."IniParser".DIRECTORY_SEPARATOR."IniParser.php";
require_once "ExternalScripts".DIRECTORY_SEPARATOR."TriggerHappyBLPreader.php";

use MPQArchive;
use BLPImage;
use Imagick;

class MapListPrint extends AbstractController
{
	protected $maplistdir = __DIR__ ."".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."MapFiles".DIRECTORY_SEPARATOR."MapList".DIRECTORY_SEPARATOR."";
	protected $target_dir = __DIR__ ."".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."MapFiles".DIRECTORY_SEPARATOR."TempDir".DIRECTORY_SEPARATOR."";
	protected $config_dir = __DIR__ ."".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."";
	
	// ENTER HERE MAP FTP SERVER IP WITH WRITE ACCESS
	protected $FTP_USER="mapdatawrite";
    protected $FTP_PASSWORD="mapdatawrite223";
    protected $FTP_IP="127.0.0.1";
    protected $FTP_PORT=21;
    protected $FTP_DIR="";
	
	function truepath($path){
		// whether $path is unix or not
		$unipath=strlen($path)==0 || $path{0}!='/';
		// attempts to detect if path is relative in which case, add cwd
		if(strpos($path,':')===false && $unipath)
			$path=getcwd().DIRECTORY_SEPARATOR.$path;
		// resolve path parts (single dot, double dot and double delimiters)
		$path = str_replace(array('/', '\\'), DIRECTORY_SEPARATOR, $path);
		$parts = array_filter(explode(DIRECTORY_SEPARATOR, $path), 'strlen');
		$absolutes = array();
		foreach ($parts as $part) {
			if ('.'  == $part) continue;
			if ('..' == $part) {
				array_pop($absolutes);
			} else {
				$absolutes[] = $part;
			}
		}
		$path=implode(DIRECTORY_SEPARATOR, $absolutes);
		// resolve any symlinks
		if(file_exists($path) && linkinfo($path)>0)$path=readlink($path);
		// put initial separator that could have been lost
		$path=!$unipath ? '/'.$path : $path;
		return $path;
	}
	
	function UpdateFtpSettings(  )
	{
		$ini_array = parse_ini_file($this->config_dir."config.ini", true);
		$this->FTP_USER=$ini_array['GENERAL']['FTP_USER'];
		$this->FTP_PASSWORD=$ini_array['GENERAL']['FTP_PASSWORD'];
		$this->FTP_IP=$ini_array['GENERAL']['FTP_IP'];
		$this->FTP_PORT=$ini_array['GENERAL']['FTP_PORT'];
		$this->FTP_DIR=$ini_array['GENERAL']['FTP_DIR'];
	}
		
	function GetOkayMapCategory($category)
	{
		if ($category != "Other" &&
			$category != "Melee" &&
			$category != "Hero_Defense" &&
			$category != "Hero_Arena" &&
			$category != "Tower_Defense" &&
			$category != "Castle_Defense" &&
			$category != "MOBA" &&
			$category != "RPG" &&
			$category != "Anime" &&
			$category != "Escape")
			{
				return "Other";
			}
		return $category;
	}

	
	function W3XcolorToHtml($war3string)
	{
		$war3string = preg_replace('/\|c\w\w(\w\w\w\w\w\w)/','<span style="color:#$1">',$war3string);
		$war3string = preg_replace('/\|r/','</span>',$war3string);
		return $war3string;
	}

	function DeleteW3XcolorFromText($war3string)
	{
		$war3string = preg_replace('/\|c\w\w\w\w\w\w\w\w/','',$war3string);
		$war3string = preg_replace('/\|r/','',$war3string);
		return $war3string;
	}

	public function actionUploadmaplistallmapList()
	{
		$visitor = \XF::visitor(); 
		
		$mapsperpage = 15;
		
		$mapcount = WarcisMapDB::getMapsCount( ); // 10
		
		$pagecount = intval(floor ($mapcount / $mapsperpage - 0.1)); // 10
		
		$startpage = 0;
		
		$pages = [0];
		
		if ($pagecount < 0)
		{
			$pagecount = 0;
		}
	
	
		if (!empty($_GET["page"]) &&  is_numeric($_GET["page"]))
		{
			$startpage = $_GET["page"];
		}
		
		if ($startpage < 0)
		{
			$startpage = 0;
		}
		else if ($startpage >= $pagecount)
		{
			$startpage = $pagecount;
		}
		
		if ($pagecount <= 3 || $startpage <= 1)
		{
			$pages = range(0,$pagecount > 2 ? 2 : $pagecount);
		}
		else
		{
			$pages = range($startpage - 1,$startpage + 1 >= $pagecount ? $startpage : $startpage + 1 );
		}
		

		
		if (!$visitor->user_id || !$visitor->username)
		{
			$OutContext = "You not registered. Please login for download or preview map.";
		}
		else 
		{
			$OutContext = "Hello,".$visitor->username.". You can see here all ($mapcount) maps.";
		}
		
		print( "Start:" . ($startpage * $mapsperpage) );
		print( "Count:" .  $mapsperpage );
		
		$viewParams = [
			'OutContext' => $OutContext,
			'maps' => WarcisMapDB::getMapsFromRange(($startpage * $mapsperpage),$mapsperpage),
			'lastpage' => $pagecount,
			'startpages' => $pages
		]; 
		
		return $this->view('XF:MapListPrint\Allmaplist', 'allmaplist', $viewParams);
	}
	
	
	public function actionUploadmaplist()
	{
		return $this->actionUploadmaplistallmapList( );
	}
		
	public function actionUploadmaplistupdate()
	{
		$this->UpdateFtpSettings();
		$visitor = \XF::visitor(); 
		$viewParams = [ ];
		if (!$visitor->user_id || !$visitor->username)
		{
			$viewParams = [
				'OutContext' => "You not registered.",
				'mapid' => null
			]; 
		}
		else if (!$visitor->is_moderator)
		{
			$viewParams = [
				'OutContext' => "Added to log. User ".$visitor->username." without moderate privileges try to use force update map function!.",
				'mapid' => null
			]; 
		}
		else 
		{
			$pvpgnBot_my = new PvPGNBot( );
			$result = $pvpgnBot_my->SendCommand( "/reloadmaplist ".$this->FTP_IP." ".$this->FTP_PORT." ".$this->FTP_USER." ".$this->FTP_PASSWORD." ".$this->FTP_DIR);
			print("Maplist updated!");
		}
		return $this->actionUploadmaplistallmapList( );
	}
	
	public function actionUploadmaplistdownload()
	{
		$this->UpdateFtpSettings( );
		
		$visitor = \XF::visitor(); 
		$viewParams = [ ];
		if (!$visitor->user_id || !$visitor->username)
		{
			$viewParams = [
				'OutContext' => "You not registered.",
				'mapid' => null
			]; 
		}
		else if( isset($_POST["mapid"]) && $_POST["mapid"] > 0) 
		{
			$viewParams = [
				'OutContext' => "Hello,".$visitor->username.". Please wait few seconds...",
				'mapid' => $_POST["mapid"]
			]; 
		}
		else 
		{
			$viewParams = [
				'OutContext' => "Please use link from Maps list for download.",
				'mapid' => null
			]; 
		}
		
		$mapid = $_POST["mapid"];
		
		if (isset($_POST["downloadmap"]) )
		{
			$curmap = WarcisMapDB::findMapById($mapid);
			
			$ftpconn = SUPERFTP::getFtpConnection($this->FTP_IP,$this->FTP_PORT,$this->FTP_USER,$this->FTP_PASSWORD,$this->FTP_DIR);
										
			if (!$ftpconn || !$curmap)
			{
				$viewParams['OutContext'] = "Error. Map not found or bad FTP connection." ;
			}
			else 
			{
				SUPERFTP::getDownloadFileToBrowser( $ftpconn, $curmap['filename']);
				/*$mapfile = SUPERFTP::getFtpFileContentsWithSize($ftpconn, $curmap['filename']);
				ftp_close($ftpconn);
				if (!$mapfile)
				{
					$viewParams['OutContext'] = "Error. File not found." ;
				}
				
				header('Content-Description: File Transfer');
				header('Content-Type: application/octet-stream');
				header('Content-Disposition: attachment; filename='.$curmap['filename']);
				header('Content-Transfer-Encoding: binary');
				header('Expires: 0');
				header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
				header('Pragma: public');
				header('Content-Length: ' . $mapfile['size']); 
				
				echo $mapfile['data'];
				exit( );*/
				
			}
			
			
		}
		else if (isset($_POST["deletemap"]) )
		{
			
			
			
			$viewParams['OutContext'] = "Try to delete map";
			
			$curmap = WarcisMapDB::findMapById($mapid);
			$ftpconn = SUPERFTP::getFtpConnection($this->FTP_IP,$this->FTP_PORT,$this->FTP_USER,$this->FTP_PASSWORD,$this->FTP_DIR);
										
			if (!$ftpconn || !$curmap)
			{
				$viewParams['OutContext'] = "Error. Map not found or bad FTP connection." ;
			}
			else 
			{
				if ($visitor->is_moderator || $curmap['user_id'] == $visitor->user_id)
				{
					
					WarcisMapDB::deleteMapById( $mapid );
					
					$deletemapresult = SUPERFTP::delFtpFile($ftpconn,$curmap['filename']);
					$deletemapiniresult = SUPERFTP::delFtpFile($ftpconn,$curmap['filename'].".ini");
					if (!$deletemapiniresult)
					{
						$deletemapiniresult = SUPERFTP::delFtpFile($ftpconn,$curmap['filename'].".in");
					}
					
					if (!$deletemapiniresult || !$deletemapresult)
					{
						$viewParams['OutContext'] = "Map deleted but with some errors." ;
					}
					else
					{
						$viewParams['OutContext'] = "Map success deleted." ;
					}
					
					$pvpgnBot_my = new PvPGNBot( );
					$result = $pvpgnBot_my->SendCommand( "/reloadmaplist ".$this->FTP_IP." ".$this->FTP_PORT." ".$this->FTP_USER." ".$this->FTP_PASSWORD." ".$this->FTP_DIR );
												
											
				}
				else
				{
					$viewParams['OutContext'] = "Sorry. No access. Added to log." ;
				}
			}
			
			ftp_close($ftpconn);
		}
		else if (isset($_POST["previewmap"]) )
		{
			$viewParams['OutContext'] = "Preview map";
			
			$curmap = WarcisMapDB::findMapById($mapid);						
			if ( !$curmap )
			{
				$viewParams['OutContext'] = "Error. Map not found ." ;
			}
			else 
			{
				$usersplayingmap = WarcraftStats::GetWarcraftAccountsByMapCode($curmap['hostcmd']);
				$viewParams = [
					'OutContext' => "Preview map.",
					'map' => $curmap,
					'usersplayingmap' => $usersplayingmap
				]; 
			}
		
			return $this->view('XF:MapListPrint\Download', 'maplistpreview', $viewParams);
		}
		 //print_r($_POST);	
		return $this->view('XF:MapListPrint\Download', 'maplistdownload', $viewParams);
	}
	
	public function actionUploadmaplistmymapList()
	{
		$visitor = \XF::visitor(); 
		if (!$visitor->user_id || !$visitor->username)
		{
			$viewParams = [
				'OutContext' => "You not registered.",
				'maps' => null
			]; 
			return $this->view('XF:MapListPrint\Mymaplist', 'mymaplist', $viewParams);
		}
		
		$mapsperpage = 20;
		
		$mapcount = WarcisMapDB::getMapsCountByParam("user_id",$visitor->user_id ); 
		
		$pagecount = intval(floor ($mapcount / $mapsperpage - 0.1)); 
		
		$startpage = 0;
		
		$pages = [0];
		
		if ($pagecount < 0)
		{
			$pagecount = 0;
		}
	
		if (!empty($_GET["page"]) &&  is_numeric($_GET["page"]))
		{
			$startpage = $_GET["page"];
		}
		
		if ($startpage < 0)
		{
			$startpage = 0;
		}
		else if ($startpage >= $pagecount)
		{
			$startpage = $pagecount;
		}
		
		if ($pagecount <= 3 || $startpage <= 1)
		{
			$pages = range(0,$pagecount > 2 ? 2 : $pagecount);
		}
		else
		{
			$pages = range($startpage - 1,$startpage + 1 >= $pagecount ? $startpage : $startpage + 1 );
		}
		
		
		if (!$visitor->user_id || !$visitor->username)
		{
			$OutContext = "You not registered. Please login for download or preview map.";
		}
		else 
		{
			$OutContext = "Hello,".$visitor->username.". You can see here your ($mapcount) maps.";
		}

		if ($visitor->user_id && $mapsperpage > 0)
			$maps = WarcisMapDB::getMapsFromRangeByParam("user_id",$visitor->user_id,($startpage * $mapsperpage),$mapsperpage);
		else 
			$maps = null;

		
		$viewParams = [
			'OutContext' => $OutContext,
			'maps' => $maps,
			'lastpage' => $pagecount,
			'startpages' => $pages
		]; 
		
		return $this->view('XF:MapListPrint\Mymaplist', 'mymaplist', $viewParams);
	}
	public function actionUploadmapliststatsmapList()
	{
		$visitor = \XF::visitor(); 
		$viewParams = [ ];
		if (!$visitor->user_id || !$visitor->username)
		{
			$viewParams = [
				'OutContext' => "You not registered",
			]; 
		}
		else 
		{
			$viewParams = [
				'OutContext' => "Hello3,".$visitor->username,
			]; 
		}
		return $this->view('XF:MapListPrint\Statsmaplist', 'statsmaplist', $viewParams);
	}

	public function actionUploadmaplistuploadmapList()
	{
		$logfile =  $this->target_dir."MapLogs.txt";
				
		$this->UpdateFtpSettings( );
		
		MPQArchive::$debugShowTables = false;

	
		$visitor = \XF::visitor(); 
		$viewParams = [ ];
		if (!$visitor->user_id || !$visitor->username)
		{
				file_put_contents($logfile,"bad\n",FILE_APPEND);
		
			$viewParams = [
				'OutContext' => "Need register for upload map!",
				'notuploaded' => false,
				'error' => true,
				'ErrorContext' =>  "Need register for upload map!"
			]; 
		}
		else 
		{
				
			$viewParams = [
				'OutContext' => "Hello,".$visitor->username.". Enter all values and press Upload button for add new map",
				'notuploaded' => true,
				'error' => false
			]; 
			
		
			if(isset($_POST["submit"]) ) 
			{
				
				if (strlen($_POST["hostcode"]) > 0 )
				{
					
					$viewParams = [
					'OutContext' => "Hello4,".$visitor->username.". You try to upload map!",
					'notuploaded' => false,
					'error' => false,
					]; 
					
				
					$uploadfile = basename($_FILES["fileToUpload"]["name"]);
				
					$target_file = $this->target_dir.$uploadfile ;
					
					$uploadOk = false;
					$imageFileType = " ";

					$filesize = 0;
					$url = " ";

					try
					{

					if (isset($_POST["URLADDR"]) && strlen ($_POST["URLADDR"]) > 3)
					{
						$url = urldecode($_POST["URLADDR"]);
						$uploadfile = basename($url);
						$target_file = $this->target_dir.$uploadfile;
						$viewParams['OutContext'] = 'UploadFile:'.$target_file;
					}

					$imageFileType = pathinfo($target_file,PATHINFO_EXTENSION);


					if (isset($_POST["URLADDR"]) && strlen ($_POST["URLADDR"]) > 3)
					{
						if (filter_var($url, FILTER_VALIDATE_URL) === FALSE) {
							if (file_exists($url))
							{
								$filesize = filesize ($url);
							}
							else 
							{
								try
								{
									$head = array_change_key_case(get_headers($url, TRUE));
									$filesize = $head['content-length'];
									}
								catch (Exception $e)
								{
									die ($e->getMessage());
								}
							}
						}
						else 
						{
							$head = array_change_key_case(get_headers($url, TRUE));
							$filesize = $head['content-length'];
						}

					}
					else
					{
						$filesize = $_FILES["fileToUpload"]["size"];
					}

					if (file_exists($target_file))
					{
						unlink($target_file);
					}

					//if (file_exists($this->maplistdir.$uploadfile)) {
					//	unlink($this->maplistdir.$uploadfile);
					//}
					 if ($filesize > 200000000 || $filesize < 10000) {
							$viewParams = [
								'OutContext' => "Need register for upload map!",
								'notuploaded' => false,
								'error' => true,
								'ErrorContext' =>  "... Sorry, your file is bad size."
							]; 
					}
					else if($imageFileType != "w3x" && $imageFileType != "w3m"  ) {
					
						$viewParams = [
								'OutContext' => "Need register for upload map!",
								'notuploaded' => false,
								'error' => true,
								'ErrorContext' =>  "only W3X, W3M files are allowed."
							]; 
					}
					else 
					{

						if (!(isset($_POST["URLADDR"]) && strlen ($_POST["URLADDR"]) > 3) && move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
							//echo "The file ". $uploadfile. " has been uploaded with size:".$filesize;
							$uploadOk = true;
						} 
						else if (isset($_POST["URLADDR"]) && strlen ($_POST["URLADDR"]) > 3)
						{
							file_put_contents($target_file, fopen($url, 'r'));
							//echo "The file ". $uploadfile . " has been uploaded with size:".$filesize;
							$uploadOk = true;
						}
						else {
							$viewParams = [
								'OutContext' => "Need register for upload map!",
								'notuploaded' => false,
								'error' => true,
								'ErrorContext' =>  "Sorry, there was an error uploading your file with size:".$filesize
							]; 
						}
					}
					
					}
					catch ( Exception $e ) 
					{	
						$viewParams = [
						'OutContext' => "Need register for upload map!",
						'notuploaded' => false,
						'error' => true,
						'ErrorContext' =>  "Fatal error while file upload: ".$e->getMessage()
					]; 
					
					}
					
					
					if ($uploadOk)
					{
						try{
							$memhackwithdllfeatures = false;
							
							$mpq = new MPQArchive($target_file, /*debug=*/false);
						
							$scriptfile = ($mpq->hasFile("war3map.j") ? "war3map.j" : ($mpq->hasFile("Scripts\\war3map.j") ? "Scripts\\war3map.j" : "scripts\\war3map.j"));
							$resultscriptfile = $mpq->readFile($scriptfile);
							$map = null;
							switch($mpq->getType())
							{   
								// Warcraft III
								case MPQArchive::TYPE_WC3MAP:
								$map = $mpq->getGameData();
								break; 
								default:
								break;
							}
						
							$map->parseData();
							
						
							$mapname = $this->DeleteW3XcolorFromText(preg_replace('/[\x00-\x1F\x7F\xA0]/u', ' ', (strlen($map->getName()) > 0 ? $map->getName() : $uploadfile)));
							$author =  $this->DeleteW3XcolorFromText(preg_replace('/[\x00-\x1F\x7F\xA0]/u', ' ', (strlen($map->getAuthor()) > 0 ? $map->getAuthor() : "Blizzard/Unknown")));
							$description = $this->DeleteW3XcolorFromText(preg_replace('/[\x00-\x1F\x7F\xA0]/u', ' ', (strlen($map->getDescription()) > 0 ? $map->getDescription() : $mapname." by ".$author)));
							
							if (strlen($description) > 500)
							{
								$description = substr($description,0,500)."...";
							}
							
							if (strlen($author) > 100)
							{
								$author = substr($author,0,100)."...";
							}
							
							if (strlen($mapname) > 250)
							{
								$mapname = substr($mapname,0,250)."...";
							}
							
							
							if ($map && $resultscriptfile )
							{
								if (preg_match('/native\s+MergeUnits/',$resultscriptfile))
								{
									$memhackwithdllfeatures = true;
								}
								if (preg_match('/native\s+IgnoredUnits/',$resultscriptfile))
								{
									$memhackwithdllfeatures = true;
								}
							
								if ($memhackwithdllfeatures && !$visitor->is_moderator)
								{
									$viewParams = [
										'OutContext' => "Need register for upload map!",
										'notuploaded' => false,
										'error' => true,
										'ErrorContext' =>  "Sorry, you need admin privileges for upload memhack map."
									]; 
									
								}
								else 
								{
									$mapcrc32 = hexdec(hash("crc32b", $resultscriptfile));
									$hostcmd = preg_replace('/[\x00-\x1F\x7F\xA0]/u', '_',$_POST["hostcode"]);
									if (strlen($hostcmd) > 40)
									{
										$hostcmd = substr($hostcmd,0,39);
									}
									if (strlen($hostcmd) < 2)
									{
										$viewParams = [
										'OutContext' => "Need register for upload map!",
										'notuploaded' => false,
										'error' => true,
										'ErrorContext' =>  "Fatal error while file upload: bad host command len"
										];
									}
									else 
									{
										$dstfile = $this->maplistdir.$uploadfile;
				
										if (WarcisMapDB::findMapByHostCmd($hostcmd))
										{
											$viewParams = [
												'OutContext' => "Need register for upload map!",
												'notuploaded' => false,
												'error' => true,
												'ErrorContext' =>  "Error. Host command used in another map."
												];
										}
										else if (WarcisMapDB::findMapByCrc32($mapcrc32))
										{
											$viewParams = [
												'OutContext' => "Need register for upload map!",
												'notuploaded' => false,
												'error' => true,
												'ErrorContext' =>  "Error. Map already uploaded by another user."
												];
										}
										else 
										{
											if (file_exists($dstfile))
											{
												unlink($dstfile);
											}
											if (file_exists($target_file))
											{
												rename($target_file, $dstfile);
											}
											else 
											{
												$viewParams = [
												'OutContext' => "Need register for upload map!",
												'notuploaded' => false,
												'error' => true,
												'ErrorContext' =>  "Unknown error. File uploaded but not exist"
												];
											}
											
											$ftpconn = SUPERFTP::getFtpConnection($this->FTP_IP,$this->FTP_PORT,$this->FTP_USER,$this->FTP_PASSWORD,$this->FTP_DIR);
											
											if (!$ftpconn)
											{
												$viewParams = [
												'OutContext' => "Need register for upload map!",
												'notuploaded' => false,
												'error' => true,
												'ErrorContext' =>  "CAN'T CONNECT TO FTP"
												];
												ftp_close($ftpconn);
											}
											else 
											{
												if (SUPERFTP::ifFileExistsAndNoZero($ftpconn,$uploadfile))
												{
													$viewParams = [
													'OutContext' => "Need register for upload map!",
													'notuploaded' => false,
													'error' => true,
													'ErrorContext' =>  "Please use another filename."
													];
													ftp_close($ftpconn);
													
												}
												else 
												{
												
												$visitor = \XF::visitor();			
												$user_id = $visitor['user_id'];		
												$username = $visitor['username'];		
											
												
												
												$mapCategory = $this->GetOkayMapCategory($_POST["mapCategory"]);
												//printf("Bot and server maplist updated!");
												$outconfig = " \n"."[MapInfo]\n"."Name=".$mapname;
												$outconfig .= "\nHostCmd=".$hostcmd."\nBotPath=".$uploadfile;
												$outconfig .= "\nScriptCrc32=".$mapcrc32."\nMapCategory=".$mapCategory;
												$outconfig .= "\nUploadedBy=".$username;
												
												//$outconfig .= "\n;Generated by MapUpload script! Next section used only for PrintMapList.\n[PrintMapListInfo]\n";
												//$outconfig .= "Downloads=0\nRating=10\nAuthor=".$author."\nDescription=".$description."\n";
												
												
												
												if (file_exists($dstfile.'.ini'))
												{
													unlink($dstfile.'.ini');
												}
												
												file_put_contents($dstfile.'.ini', $outconfig);
												
											
											
												
												$uploadmapresult = SUPERFTP::setFtpFile($ftpconn,$uploadfile,$dstfile);
												$uploadmapconfigresult = SUPERFTP::setFtpFile($ftpconn,$uploadfile.'.ini',$dstfile.'.ini');
												ftp_close($ftpconn);
												WarcisMapDB::addNewMap( $hostcmd, $mapname, $description, $description, $mapCategory, $author, $user_id, $username, false, $uploadfile, $mapcrc32);
													
														
												if (file_exists($dstfile.'.ini'))
												{
													unlink($dstfile.'.ini');
												}
												
												//print("Start generate icons");
												//MapIconExtractor::SaveMapIconsToDir($dstfile,$hostcmd);
												//print("end generate icons");
												if (file_exists($dstfile))
												{
													unlink($dstfile);
												}
												
												
												if (!$uploadmapresult || !$uploadmapconfigresult)
												{
													$viewParams = [
														'OutContext' => "Need register for upload map!",
														'notuploaded' => false,
														'error' => true,
														'ErrorContext' =>  "CAN'T UPLOAD MAP TO REMOTE FTP"
														];
												}
												else 
												{
													$filepath = $this->maplistdir.$uploadfile;
					
												
													$war3minimapblp = /*$mpq->hasFile("war3mapMap.blp") ?*/ $mpq->readFile("war3mapMap.blp");													//: null;
													
													if ($war3minimapblp)
													{
														file_put_contents ($filepath.'Minimap.blp', $war3minimapblp); 
														$blp_image1 = new BLPImage($this->truepath($filepath.'Minimap.blp')); 
														$image1 = $blp_image1->image(); 
														$image1->ScaleImage (256,256);
														$image1->setImageFormat ('jpeg');
														$image1->setImageCompressionQuality(94);
														//echo ($this->truepath($filepath.'Minimap.png'));
														$image1->writeImage($this->truepath($filepath.'Minimap.jpg'));
														if (file_exists($filepath.'Minimap.blp'))
														{
															unlink($filepath.'Minimap.blp');
														}
													}
													else 
													{
														$war3minimaptga = /*$mpq->hasFile("war3mapMap.tga") ?*/ $mpq->readFile("war3mapMap.tga");// : null;
													
														if ($war3minimaptga)
														{
															file_put_contents ($filepath.'Minimap.tga', $war3minimaptga); 
															$image1 = new Imagick($this->truepath($filepath.'Minimap.tga'));
															$image1->ScaleImage (256,256);;
															$image1->setImageFormat ('jpeg');
															$image1->setImageCompressionQuality(94);
														//	echo ($this->truepath($filepath.'Minimap.png'));
															$image1->writeImage($this->truepath($filepath.'Minimap.jpg'));
															if (file_exists($filepath.'Minimap.tga'))
															{
																unlink($filepath.'Minimap.tga');
															}
														}
														else 
														{
															if (file_exists(__DIR__ ."".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."MapFiles".DIRECTORY_SEPARATOR."bad_minimap.jpg"))
															{
																copy(__DIR__ ."".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."MapFiles".DIRECTORY_SEPARATOR."bad_minimap.jpg",$this->truepath($filepath.'Minimap.jpg'));
															}
															else 
															{
																print("FILE:"."".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."MapFiles".DIRECTORY_SEPARATOR."bad_minimap.jpg"." NOT FOUND!");
															}
														}
													}
												
													$mapPreviewblp = /*$mpq->hasFile("war3mapPreview.blp") ? */$mpq->readFile("war3mapPreview.blp"); //: null;
													
													if ($mapPreviewblp )
													{
														file_put_contents ($filepath.'Preview.blp', $mapPreviewblp); 
														$blp_image2 = new BLPImage($this->truepath($filepath.'Preview.blp')); 
														$image1 = $blp_image2->image(); 
														$image1->ScaleImage (256,256);
														$image1->setImageFormat ('jpeg');
														$image1->setImageCompressionQuality(90);
														$image1->writeImage($this->truepath($filepath.'Preview.jpg'));
														if (file_exists($filepath.'Preview.blp'))
														{
															unlink($filepath.'Preview.blp');
														}
													}
													else
													{
														$mapPreviewtga = /*$mpq->hasFile("war3mapPreview.tga") ?*/ $mpq->readFile("war3mapPreview.tga");// : null;
													
														if ($mapPreviewtga )
														{
															file_put_contents ($filepath.'Preview.tga', $mapPreviewtga); 
															$image1 = new Imagick($this->truepath($filepath.'Preview.tga'));
															$image1->ScaleImage (256,256);
															$image1->setImageFormat ('jpeg');
															$image1->setImageCompressionQuality(90);
															$image1->writeImage($this->truepath($filepath.'Preview.jpg'));
															if (file_exists($filepath.'Preview.tga'))
															{
																unlink($filepath.'Preview.tga');
															}
														}
														else 
														{	
															if (file_exists(__DIR__ ."".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."MapFiles".DIRECTORY_SEPARATOR."bad_preview.jpg"))
															{
																copy(__DIR__ ."".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."MapFiles".DIRECTORY_SEPARATOR."bad_preview.jpg",$this->truepath($filepath.'Preview.jpg'));
															}
															else 
															{
																print("FILE:"."".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."..".DIRECTORY_SEPARATOR."MapFiles".DIRECTORY_SEPARATOR."bad_preview.jpg"." NOT FOUND!");
															}
														}
													}
											
													
														
													
													$pvpgnBot_my = new PvPGNBot( );
													$result = $pvpgnBot_my->SendCommand( "/reloadmaplist ".$this->FTP_IP." ".$this->FTP_PORT." ".$this->FTP_USER." ".$this->FTP_PASSWORD." ".$this->FTP_DIR );
													
												
													$viewParams = [
													'OutContext' => "Hello4,".$visitor->username.". You map success uploaded!",
													'notuploaded' => false,
													'error' => false,
													]; 
													
												}
												}
											}
										}
										
									}
								}
							}
							else 
							{
								$viewParams = [
											'OutContext' => "Need register for upload map!",
											'notuploaded' => false,
											'error' => true,
											'ErrorContext' =>  "Bad map or war3map.j not found."
											];
												
							}
							
							$mpq->close( );
						}
						catch ( Exception $e ) 
						{	
							$viewParams = [
							'OutContext' => "Need register for upload map!",
							'notuploaded' => false,
							'error' => true,
							'ErrorContext' =>  "Fatal error while file upload: ".$e->getMessage()
							]; 
						}
						catch(MPQException $error)
						{
							$viewParams = [
							'OutContext' => "Need register for upload map!",
							'notuploaded' => false,
							'error' => true,
							'ErrorContext' =>  "Fatal MPQ LIB ERROR while file upload: ". $error->getMessage()
							]; 
						}
					}
					
					
					
					if (file_exists($target_file))
					{
						unlink($target_file);
					}

					
					
					
				}
				else 
				{
					$viewParams = [
						'OutContext' => "Need enter valid options!",
						'notuploaded' => false,
						'error' => true,
						'ErrorContext' =>  "Need enter valid options map!"
					]; 
				}
			}
		}
		return $this->view('XF:MapListPrint\Uploadmaplist', 'uploadmaplist', $viewParams);
	}
}