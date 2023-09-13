<?php

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

function W3XcolorToHtml($war3string)
{
	$war3string = preg_replace('/\|c\w\w(\w\w\w\w\w\w)/','<span style="color:#$1">',$war3string);
	$war3string = preg_replace('/\|r/','</span>',$war3string);
	return $war3string;
}

require 'TriggerHappyMPQreader/mpq.php';
require 'TriggerHappyBLPreader.php';
require 'IniParser/IniParser.php';
$maplistdir = "./maplistfullnew/";
$dir = new DirectoryIterator($maplistdir);
$curid = 0;
$startid = 0;
$endid = 10000;
printf('<html><title>Warcis Map List</title><head><script type="text/javascript" src="sort-table.js"></script> <link rel="stylesheet" type="text/css" href="sort-table.css" title=""><style type="text/css">table { border: 1px solid black; border-collapse: collapse; }th, td { padding: 2px 5px; border: 1px solid black; }thead { background: #ddd; } table#demo2.js-sort-0 tbody tr td:nth-child(1), table#demo2.js-sort-1 tbody tr td:nth-child(2), table#demo2.js-sort-2 tbody tr td:nth-child(3), table#demo2.js-sort-3 tbody tr td:nth-child(4),table#demo2.js-sort-4 tbody tr td:nth-child(5), table#demo2.js-sort-5 tbody tr td:nth-child(6), table#demo2.js-sort-6 tbody tr td:nth-child(7), table#demo2.js-sort-7 tbody tr td:nth-child(8), table#demo2.js-sort-8 tbody tr td:nth-child(9), table#demo2.js-sort-9 tbody tr td:nth-child(10) {  background: #dee;}</style></head><body>');
printf('<table class="js-sort-table" id="maplisttable">');
printf('<thead><tr> <th>Preview and minimap</th> <th>Info</th> <th>Description</th> <th>Host</th> </tr> </thead> <tbody>');

foreach ($dir as $fileinfo) 
{
	if ($fileinfo->isFile())
	{
		if ($fileinfo->getExtension() != "w3x" && $fileinfo->getExtension() != "w3m")
		{
			continue;
		}
		

		if ($startid > $curid)
		{	
			$curid++;
			continue;
		}
		
		$filepath = $maplistdir.$fileinfo->getFilename();
		
		if (!file_exists($filepath.'.ini'))
		{
			echo("<br>! Not found file:".$filepath.'.ini');
			continue;
		}
		
		try
		{
			$MapConfigParser = new IniParser($filepath.'.ini');
			$MapConfig = $MapConfigParser->parse();
			if (file_exists($filepath.'Minimap.png') && file_exists($filepath.'Preview.png'))
			{
				$basefilepath = basename($filepath);
				$outdate5 = '<tr><td><img src="ShowImage/?image_name='.urlencode($basefilepath.
				'Preview.png').'"  height="256" width="256" /><img src="ShowImage/?image_name='.urlencode($basefilepath.
				'Minimap.png').'"  height="256" width="256" /></td><td width="300">Name:'.W3XcolorToHtml($MapConfig->MapInfo->Name).
				'<div/>Author:'.W3XcolorToHtml($MapConfig->PrintMapListInfo->Author).
				'<div/>Category:'.$MapConfig->MapInfo->MapCategory.'
				<div/>Upload time:'.date ("F d Y H:i:s.", filemtime($filepath)).
				'<div/>Rating:'.$MapConfig->PrintMapListInfo->Rating.
				'</td><td width="300">'.W3XcolorToHtml($MapConfig->PrintMapListInfo->Description).
				'</td><td width="300">Downloads:'.$MapConfig->PrintMapListInfo->Downloads.
				'<div/><a href="./MapDownload.php?mappath='.$basefilepath.'">'.$basefilepath.'</a><div/>Host cmd:'.$MapConfig->MapInfo->HostCmd.'</td></tr>';
				printf("%s",$outdate5);
			}
			else 
			{
			
				//print("<div/>MapName:".$MapConfig->MapInfo->Name);
				
				$mpq = new MPQArchive($filepath, /*debug=*/false);
				//$file = ($mpq->hasFile("war3map.j") ? "war3map.j" : ($mpq->hasFile("Scripts\\war3map.j") ? "Scripts\\war3map.j" : "scripts\\war3map.j"));
				//$result = $mpq->readFile($file);
			
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
				
				if ($map != null && /*$result != null &&*/ $map->parseData())
				{
					
					ob_start();
					if (!file_exists($filepath.'Minimap.png'))
					{
						$war3minimapblp = $mpq->hasFile("war3mapMap.blp") ? $mpq->readFile("war3mapMap.blp") : null;
						$war3minimaptga = $mpq->hasFile("war3mapMap.tga") ? $mpq->readFile("war3mapMap.tga") : null;

						if ($war3minimaptga != null)
						{
							file_put_contents ($filepath.'Minimap.tga', $war3minimaptga); 
							$image1 = new Imagick(truepath($filepath.'Minimap.tga'));
							$image1->ScaleImage (256,256);;
							$image1->setImageFormat ('png');
						
						//	echo (truepath($filepath.'Minimap.png'));
							$image1->writeImage(truepath($filepath.'Minimap.png'));
						}
						
						else if ($war3minimapblp != null)
						{
							file_put_contents ($filepath.'Minimap.blp', $war3minimapblp); 
							$blp_image1 = new BLPImage(truepath($filepath.'Minimap.blp')); 
							$image1 = $blp_image1->image(); 
							$image1->ScaleImage (256,256);
							$image1->setImageFormat ('png');
							
							//echo (truepath($filepath.'Minimap.png'));
							$image1->writeImage(truepath($filepath.'Minimap.png'));
						}
						else 
						{
							copy("bad_minimap.png",truepath($filepath.'Minimap.png'));
						}
						
					}
					else 
					{
						//echo("Minimap found!");
					}
					
					if (!file_exists($filepath.'Preview.png'))
					{
						$mapPreviewblp = $mpq->hasFile("war3mapPreview.blp") ? $mpq->readFile("war3mapPreview.blp") : null;
						$mapPreviewtga = $mpq->hasFile("war3mapPreview.tga") ? $mpq->readFile("war3mapPreview.tga") : null;
						
						
						
						if ($mapPreviewtga != null)
						{
							file_put_contents ($filepath.'Preview.tga', $mapPreviewtga); 
							$image1 = new Imagick(truepath($filepath.'Preview.tga'));
							$image1->ScaleImage (256,256);
							$image1->setImageFormat ('png');
							//echo (truepath($filepath.'Preview.png'));
							$image1->writeImage(truepath($filepath.'Preview.png'));
						}
						else if ($mapPreviewblp != null)
						{
							file_put_contents ($filepath.'Preview.blp', $mapPreviewblp); 
							$blp_image2 = new BLPImage(truepath($filepath.'Preview.blp')); 
							$image1 = $blp_image2->image(); 
							$image1->ScaleImage (256,256);
							$image1->setImageFormat ('png');
						//	echo (truepath($filepath.'Preview.png'));
							$image1->writeImage(truepath($filepath.'Preview.png'));
						}
						else 
						{
							copy("bad_preview.png",truepath($filepath.'Preview.png'));
						}
					}
					else 
					{
						//echo("Preview found!");
					}
					
					ob_clean();
					printf(sprintf('<tr><td><img src="'.$filepath.'Preview.png'.'"  height="256" width="256" /><img src="'.$filepath.'Minimap.png'.'"  height="256" width="256" /></td><td width="300">Name:%s<div/>Author:%s<div/>Category:%s<div/>Upload time:%s<div/>Rating:%s</td><td width="300">%s</td><td width="300">Downloads:%s<div/><a href="./MapDownload.php?mappath=%s">%s</a><div/>Host cmd:%s</td></tr>',  
					W3XcolorToHtml($MapConfig->MapInfo->Name),
					W3XcolorToHtml($MapConfig->PrintMapListInfo->Author),
					$MapConfig->MapInfo->MapCategory,
					date ("F d Y H:i:s.", filemtime($filepath)),
					$MapConfig->PrintMapListInfo->Rating,
					W3XcolorToHtml($MapConfig->PrintMapListInfo->Description),
					$MapConfig->PrintMapListInfo->Downloads,$filepath,
					$filepath,$MapConfig->MapInfo->HostCmd));
					
				}
				$mpq->close( );
			}
		}
		catch(MPQException $error)
		{
			die(nl2br("<strong>Error:</strong> " . $error->getMessage() . "\n\n" . $error));
		}
		catch(Exception $error)
		{
			die(nl2br("<strong>Error:</strong> " . $error->getMessage() . "\n\n" . $error));
		}
	
	}
}

printf('</tbody></table></body></html>');

?>