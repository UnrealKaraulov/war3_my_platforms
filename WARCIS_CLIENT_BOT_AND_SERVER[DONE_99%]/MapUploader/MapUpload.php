<?php
require 'TriggerHappyMPQreader/mpq.php';
require 'bot_reloadmaplist.php';
require 'IniParser/IniParser.php';

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

function FindValueInAllMapConfigs($pathdir, $section,$key,$value)
{
	$dir = new DirectoryIterator($pathdir);
	foreach ($dir as $fileinfo) 
	{
		if ($fileinfo->isFile())
		{
			if ($fileinfo->getExtension() != "ini" )
			{
				continue;
			}
			
			$filepath = $pathdir.$fileinfo->getFilename();
			$MapConfigParser = new IniParser($filepath);
			$MapConfig = $MapConfigParser->parse();
			if ($MapConfig->$section->$key == $value)
				return true;
		}
	}
	return false;
}


function FindValueInAllMapConfigsGivePath($pathdir, $section,$key,$value)
{
	$dir = new DirectoryIterator($pathdir);
	foreach ($dir as $fileinfo) 
	{
		if ($fileinfo->isFile())
		{
			if ($fileinfo->getExtension() != "ini" )
			{
				continue;
			}
			
			$filepath = $pathdir.$fileinfo->getFilename();
			$MapConfigParser = new IniParser($filepath);
			$MapConfig = $MapConfigParser->parse();
			if ($MapConfig->$section->$key == $value)
				return substr($filepath , 0, -4);
		}
	}
	return "";
}

if(isset($_POST["submit"]) && isset($_POST["hostcode"]) ) {
try
{
	$maplistdir = "./maplistfullnew/";
	$target_dir = "./TempDirForMapTest/";
	$uploadfile = basename($_FILES["fileToUpload"]["name"]);
	$target_file = $target_dir.$uploadfile ;
	$uploadOk = false;
	$imageFileType = " ";

	$filesize = 0;
	$url = " ";


	if (isset($_POST["URLADDR"]) && strlen ($_POST["URLADDR"]) > 3)
	{
		$url = urldecode($_POST["URLADDR"]);
		$uploadfile = basename($url);
		$target_file = $target_dir.$uploadfile;
		print ('UploadFile:'.$target_file);
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

	if (file_exists($maplistdir.$uploadfile)) {
		echo "... Sorry, file already exists.";
	}
	else if ($filesize > 200000000 || $filesize < 10000) {
		echo "... Sorry, your file is bad size.";
	}
	else if($imageFileType != "w3x" && $imageFileType != "w3m"  ) {
		echo "... Sorry, only W3X, W3M files are allowed.";
	}
	else 
	{
		
		if (!(isset($_POST["URLADDR"]) && strlen ($_POST["URLADDR"]) > 3) && move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
			echo "The file ". $uploadfile. " has been uploaded with size:".$filesize;
			$uploadOk = true;
		} 
		else if (isset($_POST["URLADDR"]) && strlen ($_POST["URLADDR"]) > 3)
		{
			file_put_contents($target_file, fopen($url, 'r'));
			echo "The file ". $uploadfile . " has been uploaded with size:".$filesize;
			$uploadOk = true;
		}
		else {
			echo "Sorry, there was an error uploading your file with size:".$filesize;
		}
	}
}
catch ( Exception $e ) 
{
	echo "<div/>Fatal error while file upload: ".$e->getMessage();
}
if ($uploadOk)
{
	try{
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
		
		
		$mapname = preg_replace('/[\x00-\x1F\x7F\xA0]/u', ' ', (strlen($map->getName()) > 0 ? $map->getName() : $uploadfile));
		$author = preg_replace('/[\x00-\x1F\x7F\xA0]/u', ' ', (strlen($map->getAuthor()) > 0 ? $map->getAuthor() : "Blizzard/Unknown"));
		$description =  preg_replace('/[\x00-\x1F\x7F\xA0]/u', ' ', (strlen($map->getDescription()) > 0 ? $map->getDescription() : $mapname." by ".$author));
		
		
		$mpq->close( );
		if ($map && $resultscriptfile  && $map)
		{
			if (!isset($_POST["SECRETCODE"]) || $_POST["SECRETCODE"] != "SuperSecretUploadMemHackCode")
			{
				if (preg_match('/native\s+MergeUnits/',$resultscriptfile))
				{
					die("You try to upload map with MemHack");
				}
				if (preg_match('/native\s+IgnoredUnits/',$resultscriptfile))
				{
					die("You try to upload map with MemHack #2");
				}
			}
			$mapcrc32 = hexdec(hash("crc32b", $resultscriptfile));
			$hostcmd = preg_replace('/[\x00-\x1F\x7F\xA0]/u', '_',$_POST["hostcode"]);
			if (strlen($hostcmd) > 40)
			{
				$hostcmd = substr($hostcmd,0,39);
			}
			if (strlen($hostcmd) < 2)
			{
				die("bad host code!");
			}
			
			$update = false;
			$dstfile = $maplistdir.$uploadfile;
			if( isset($_POST["UpdateCode"])  &&  strlen($_POST["UpdateCode"]) > 5)
			{
				$oldfilepath = FindValueInAllMapConfigsGivePath($maplistdir,"PrintMapListInfo","UpdateCode",$_POST["UpdateCode"]);
				if (strlen($oldfilepath) > 0)
				{
					$update = true;
					$dstfile = $oldfilepath;
				}
			}
			
			
			
			if (FindValueInAllMapConfigs($maplistdir,"MapInfo","HostCmd",$hostcmd) && !$update)
			{
				printf("<div/> Error! This hostcmd used in another map!");
			}
			else if (FindValueInAllMapConfigs($maplistdir,"MapInfo","ScriptCrc32",$mapcrc32))
			{
				printf("<div/> Error! This crc32 used in another map!");
			}
			else 
			{
				printf("<div/>Map file added to list!");
				
				if (file_exists($dstfile))
				{
					unlink($dstfile);
				}
				
				rename($target_file, $dstfile);
				printf("Bot and server maplist updated!");
				$outconfig = " \n"."[MapInfo]\n"."Name=".$mapname;
				$outconfig .= "\nHostCmd=".$hostcmd."\nBotPath=".$uploadfile;
				$outconfig .= "\nScriptCrc32=".$mapcrc32."\nMapCategory=".GetOkayMapCategory($_POST["mapCategory"]);
				$outconfig .= "\n;Generated by MapUpload script! Next section used only for PrintMapList.\n[PrintMapListInfo]\n";
				$outconfig .= "Downloads=0\nRating=10\nAuthor=".$author."\nDescription=".$description."\n";
				if( isset($_POST["UpdateCode"])  &&  strlen($_POST["UpdateCode"]) > 5)
				{
					$outconfig .= "UpdateCode=".$_POST["UpdateCode"];
				}
				
				if (file_exists($dstfile.'.ini'))
				{
					unlink($dstfile.'.ini');
				}
				
				file_put_contents($dstfile.'.ini', $outconfig);
				printf("Start update map list...");
				ob_start();
				$pvpgnBot_my = new pvpgnBot( );
				$pvpgnBot_my->Init( );
				ob_clean( );
				printf("SUCCESS!");
			}
		}
		else 
		{
			$uploadOk = false;
			printf("<div/>Invalid map file.".($map==null ? "badmap" : "ok").' '.($resultscriptfile == null ? "badresultfile" : "ok").' '.(!$map->parseData() ? "badparseData" : "ok").' ');
		}
		
		
	}
	catch ( Exception $e ) 
	{
		$uploadOk = false;
		echo "Fatal error #2 while file upload: ".$e->getMessage();
	}
	catch ( MPQException $error ) 
	{
		$uploadOk = false;
		echo "MPQ Fatal error while reading file: ".$e->getMessage();
	}
	if (!$uploadOk)
	{
		unlink($target_file);
	}
}



}
else 
{
	exit(header("Location: ./index.html"));
}
?>