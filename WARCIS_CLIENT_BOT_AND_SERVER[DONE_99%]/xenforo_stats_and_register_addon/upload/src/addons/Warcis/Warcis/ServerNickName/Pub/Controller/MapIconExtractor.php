<?php

namespace Warcis\ServerNickName\Pub\Controller;

use XF\Mvc\ParameterBag;
use XF\Mvc\FormAction;
use XF\Mvc\Reply\View;

require_once "ExternalScripts/TriggerHappyMPQreader/mpq.php";
require_once "ExternalScripts/IniParser/IniParser.php";
require_once "ExternalScripts/TriggerHappyBLPreader.php";

use MPQArchive;
use BLPImage;
use Imagick;

class MapIconExtractor
{
	
	public static function truepath($path){
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
	
	
	
	public static function parse_ini_string_m($str) {

		if(empty($str)) return false;

		$lines = preg_split("/\\r\\n|\\r|\\n/", $str);
		$ret = Array();
		$inside_section = false;

		foreach($lines as $line) {

			$line = trim($line);

			if(!$line || $line[0] == "#" || $line[0] == ";") 
				continue;

			if($line[0] == "[" && $endIdx = strpos($line, "]"))
			{
				$inside_section = substr($line, 1, $endIdx-1);
				continue;
			}

			if(!strpos($line, '=')) continue;

			$tmp = explode("=", $line, 2);

			if($inside_section) {

			$key = rtrim($tmp[0]);
			$value = ltrim($tmp[1]);

			if(preg_match("/^\".*\"$/", $value) || preg_match("/^'.*'$/", $value)) {
				$value = mb_substr($value, 1, mb_strlen($value) - 2);
			}

			$t = preg_match("^\[(.*?)\]^", $key, $matches);
			if(!empty($matches) && isset($matches[0])) {

			$arr_name = preg_replace('#\[(.*?)\]#is', '', $key);

			if(!isset($ret[$inside_section][$arr_name]) || !is_array($ret[$inside_section][$arr_name])) {
				$ret[$inside_section][$arr_name] = array();
			}

			if(isset($matches[1]) && !empty($matches[1])) {
				$ret[$inside_section][$arr_name][$matches[1]] = $value;
			} else {
				$ret[$inside_section][$arr_name][] = $value;
			}

			} else {
				$ret[$inside_section][trim($tmp[0])] = $value;
			}            

			} else {

				$ret[trim($tmp[0])] = ltrim($tmp[1]);

			}
		}
		return $ret;
	}

	
	public static function ConvertTgaToJpegPath( $tgadataPath , $jpgpath )
	{
		$image1 = new Imagick(self::truepath($tgadataPath));
		$image1->ScaleImage (256,256);;
		$image1->setImageFormat ('jpeg');
		$image1->setImageCompressionQuality(94);
		$image1->writeImage(self::truepath($jpgpath));
	}
	
	
	public static function ConvertBlpToJpegPath( $tgadataPath , $jpgpath )
	{
		$blp_image1 = new BLPImage(self::truepath( $tgadataPath)); 
		$image1 = $blp_image1->image(); 
		$image1->ScaleImage (256,256);
		$image1->setImageFormat ('jpeg');
		$image1->setImageCompressionQuality(94);
		$image1->writeImage(self::truepath($jpgpath));
	}
	
	
	public static function ConvertImageToJpeg( $imagepath, $jpgpath, $ext)
	{
		if ($ext == 'blp')
			self::ConvertBlpToJpegPath($imagepath,$jpgpath);
		else 
			self::ConvertTgaToJpegPath($imagepath,$jpgpath);
	}
	

	public static function ConvertTgaToJpeg( $tgadata , $jpgpath )
	{
		file_put_contents ($jpgpath.'tmp.tga', $tgadata); 
		$image1 = new Imagick(self::truepath($jpgpath.'tmp.tga'));
		$image1->ScaleImage (256,256);;
		$image1->setImageFormat ('jpeg');
		$image1->setImageCompressionQuality(94);
		$image1->writeImage(self::truepath($jpgpath));
		if (file_exists($jpgpath.'tmp.tga'))
		{
			unlink($jpgpath.'tmp.tga');
		}
	}
	
	
	public static function ConvertBlpToJpeg( $tgadata , $jpgpath )
	{
		file_put_contents ($jpgpath.'tmp.blp', $war3minimapblp); 
		$blp_image1 = new BLPImage(self::truepath($jpgpath.'tmp.blp')); 
		$image1 = $blp_image1->image(); 
		$image1->ScaleImage (256,256);
		$image1->setImageFormat ('jpeg');
		$image1->setImageCompressionQuality(94);
		$image1->writeImage(self::truepath($jpgpath));
		if (file_exists($jpgpath.'tmp.blp'))
		{
			unlink($jpgpath.'tmp.blp');
		}
	}
	
	public static function  write_ini_file($assoc_arr, $path, $has_sections=FALSE) { 
		$content = ""; 
		if ($has_sections) { 
			foreach ($assoc_arr as $key=>$elem) { 
				$content .= "[".$key."]\n"; 
				foreach ($elem as $key2=>$elem2) { 
					if(is_array($elem2)) 
					{ 
						for($i=0;$i<count($elem2);$i++) 
						{ 
							$content .= $key2."[] = ".$elem2[$i]."\n"; 
						} 
					} 
					else if($elem2=="") $content .= $key2." = \n"; 
					else $content .= $key2." = ".$elem2."\n"; 
				} 
			} 
		} 
		else { 
			foreach ($assoc_arr as $key=>$elem) { 
				if(is_array($elem)) 
				{ 
					for($i=0;$i<count($elem);$i++) 
					{ 
						$content .= $key."[] = ".$elem[$i]."\n"; 
					} 
				} 
				else if($elem=="") $content .= $key." = \n"; 
				else $content .= $key." = ".$elem."\n"; 
			} 
		} 

		if (!$handle = fopen($path, 'w')) { 
			return false; 
		}

		$success = file_put_contents($path,$content);
		fclose($handle); 

		return $success; 
	}
	/*
	
		Если файл существует вернуть путь
		
	
	
	*/
	
	public static function ExtractIconToPath( $mpq, $filename, $basedirpath, $outiconpath, $recursived = false )
	{
		$logfile = $basedirpath."/log.txt";
			
		if (file_exists($outiconpath))
			return true;

		
		if (!$recursived)
		{
			$filename = strtoupper($filename);
			
			$ext = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
			
			$DisabledIconSignature = strtoupper("Disabled\\DIS");
			$DisabledIconSignature2 = strtoupper("Disabled\\DISDIS");
			$CommandButtonsDisabledIconSignature = strtoupper("CommandButtonsDisabled\\DIS");
			
			
				
			file_put_contents($logfile, "Try to find:". $outiconpath."\n", FILE_APPEND );
			
			if (self::ExtractIconToPath($mpq,$filename,$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
			
			if (self::ExtractIconToPath($mpq,$filename.".blp",$basedirpath,$outiconpath,true))
			{	
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
			if (self::ExtractIconToPath($mpq,$filename.".tga",$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
			
			file_put_contents($logfile, "1\n", FILE_APPEND );
			
			$blpbasepath = $basedirpath."DefaultIconFiles/".$filename;
			
			$blpbasepathnew = str_replace("\\","/",$blpbasepath) ;
			
					
			if (file_exists($blpbasepathnew))
			{
				self::ConvertImageToJpeg($blpbasepathnew, $outiconpath, $ext);
				return true;
			}
			
			if (file_exists($blpbasepathnew.".blp"))
			{
				self::ConvertImageToJpeg($blpbasepathnew.".blp", $outiconpath, 'blp');
				return true;
			}
			
		
			if (file_exists($blpbasepathnew.".tga"))
			{
				self::ConvertImageToJpeg($blpbasepathnew.".tga", $outiconpath, 'tga');
				return true;
			}
			
					
			file_put_contents($logfile, "2\n", FILE_APPEND );
			
			if (self::ExtractIconToPath($mpq,str_replace($DisabledIconSignature,"\\",$filename),$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
			
			if (self::ExtractIconToPath($mpq,str_replace($DisabledIconSignature,"\\",$filename)."blp",$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
			
			if (self::ExtractIconToPath($mpq,str_replace($DisabledIconSignature,"\\",$filename)."tga",$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
		
			
			$blpbasepathnew = $blpbasepath ;
			$blpbasepathnew = str_replace($DisabledIconSignature,"\\",$blpbasepathnew);
			$blpbasepathnew = str_replace("\\","/",$blpbasepathnew) ;
			
					
			file_put_contents($logfile, "3\n", FILE_APPEND );
			
					
			if (file_exists($blpbasepathnew))
			{
				self::ConvertImageToJpeg($blpbasepathnew, $outiconpath, $ext);
				return true;
			}
			
			if (file_exists($blpbasepathnew.".BLP"))
			{
				self::ConvertImageToJpeg($blpbasepathnew.".blp", $outiconpath, 'blp');
				return true;
			}
			
		
			if (file_exists($blpbasepathnew.".TGA"))
			{
				self::ConvertImageToJpeg($blpbasepathnew.".tga", $outiconpath, 'tga');
				return true;
			}
			
					
			file_put_contents($logfile, "4\n", FILE_APPEND );
			
			
			if (self::ExtractIconToPath($mpq,str_replace($DisabledIconSignature2,"\\",$filename),$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
			
			if (self::ExtractIconToPath($mpq,str_replace($DisabledIconSignature2,"\\",$filename)."blp",$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
			if (self::ExtractIconToPath($mpq,str_replace($DisabledIconSignature2,"\\",$filename)."tga",$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
			
					
			file_put_contents($logfile, "5\n", FILE_APPEND );
			
			
			$blpbasepathnew = $blpbasepath ;
			$blpbasepathnew = str_replace($DisabledIconSignature2,"\\",$blpbasepathnew);
			$blpbasepathnew = strtoupper(str_replace("\\","/",$blpbasepathnew)) ;
			
				
						
				if (file_exists($blpbasepathnew))
			{
				self::ConvertImageToJpeg($blpbasepathnew, $outiconpath, $ext);
				return true;
			}
			
			if (file_exists($blpbasepathnew.".BLP"))
			{
				self::ConvertImageToJpeg($blpbasepathnew.".blp", $outiconpath, 'blp');
				return true;
			}
			
		
			if (file_exists($blpbasepathnew.".TGA"))
			{
				self::ConvertImageToJpeg($blpbasepathnew.".tga", $outiconpath, 'tga');
				return true;
			}
			
			
					
			file_put_contents($logfile, "6\n", FILE_APPEND );
			
			if (self::ExtractIconToPath($mpq,str_replace($CommandButtonsDisabledIconSignature,"PassiveButtons\\",$filename),$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
			if (self::ExtractIconToPath($mpq,str_replace($CommandButtonsDisabledIconSignature,"PassiveButtons\\",$filename)."blp",$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
			
			if (self::ExtractIconToPath($mpq,str_replace($CommandButtonsDisabledIconSignature,"PassiveButtons\\",$filename)."tga",$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
			
			$blpbasepathnew = $blpbasepath ;
			$blpbasepathnew = str_replace($CommandButtonsDisabledIconSignature,"PassiveButtons\\",$blpbasepathnew);
			$blpbasepathnew = strtoupper(str_replace("\\","/",$blpbasepathnew)) ;
			
				
								
			file_put_contents($logfile, "7\n", FILE_APPEND );
			
				if (file_exists($blpbasepathnew))
			{
				self::ConvertImageToJpeg($blpbasepathnew, $outiconpath, $ext);
				return true;
			}
			
			if (file_exists($blpbasepathnew.".BLP"))
			{
				self::ConvertImageToJpeg($blpbasepathnew.".blp", $outiconpath, 'blp');
				return true;
			}
			
		
			if (file_exists($blpbasepathnew.".TGA"))
			{
				self::ConvertImageToJpeg($blpbasepathnew.".tga", $outiconpath, 'tga');
				return true;
			}
			
						
			file_put_contents($logfile, "8\n", FILE_APPEND );
			
			if (self::ExtractIconToPath($mpq,str_replace($CommandButtonsDisabledIconSignature,"AutoCastButtons\\",$filename),$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
			
			if (self::ExtractIconToPath($mpq,str_replace($CommandButtonsDisabledIconSignature,"AutoCastButtons\\",$filename)."blp",$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
					
			if (self::ExtractIconToPath($mpq,str_replace($CommandButtonsDisabledIconSignature,"AutoCastButtons\\",$filename)."tga",$basedirpath,$outiconpath,true))
			{
				$war3mpq2->close( );
				$war3xmpq2->close( );
				return true;
			}
					
			file_put_contents($logfile, "9\n", FILE_APPEND );
			
			
			$blpbasepathnew = $blpbasepath ;
			$blpbasepathnew = str_replace($CommandButtonsDisabledIconSignature,"AutoCastButtons\\",$blpbasepathnew);
			$blpbasepathnew = strtoupper(str_replace("\\","/",$blpbasepathnew)) ;
			
				
			if (file_exists($blpbasepathnew))
			{
				self::ConvertImageToJpeg($blpbasepathnew, $outiconpath, $ext);
				return true;
			}
			
			if (file_exists($blpbasepathnew.".BLP"))
			{
				self::ConvertImageToJpeg($blpbasepathnew.".blp", $outiconpath, 'blp');
				return true;
			}
			
		
			if (file_exists($blpbasepathnew.".TGA"))
			{
				self::ConvertImageToJpeg($blpbasepathnew.".tga", $outiconpath, 'tga');
				return true;
			}
			
				
			file_put_contents($logfile, "NOT FOUND\n", FILE_APPEND );
			
			
			return false;
		}
		else 
		{
		
			$filedata = $mpq->readFile($filename);
		
			if (!$filedata)
			{
				file_put_contents($logfile, "Not found:$filename\n", FILE_APPEND );
			
				return false;
			}
			file_put_contents($logfile, "Found:$filename\n", FILE_APPEND );
			
			file_put_contents($outiconpath,$filedata);
			return true;
		}
		
	}
	
	public static function SaveMapIconsToDir( $mapfile, $mapcode )
	{
		
		MPQArchive::$debugShowTables = false;
		
		$mapicondir = __DIR__ ."/../../MapFiles/IconFiles";

		$objtxtdatafiles = array(   'Units\CampaignAbilityFunc.txt',
									'Units\CampaignAbilityStrings.txt','Units\CampaignUnitFunc.txt',
									'Units\CampaignUnitStrings.txt','Units\CampaignUpgradeFunc.txt',
									'Units\CampaignUpgradeStrings.txt','Units\CommandFunc.txt',
									'Units\CommandStrings.txt','Units\CommonAbilityFunc.txt',
									'Units\CommonAbilityStrings.txt','Units\HumanAbilityFunc.txt',
									'Units\HumanAbilityStrings.txt','Units\HumanUnitFunc.txt',
									'Units\HumanUnitStrings.txt','Units\HumanUpgradeFunc.txt',
									'Units\HumanUpgradeStrings.txt','Units\ItemAbilityFunc.txt',
									'Units\ItemAbilityStrings.txt','Units\ItemFunc.txt',
									'Units\ItemStrings.txt','Units\NeutralAbilityFunc.txt',
									'Units\NeutralAbilityStrings.txt','Units\NeutralUnitFunc.txt',
									'Units\NeutralUnitStrings.txt','Units\NeutralUpgradeFunc.txt',
									'Units\NeutralUpgradeStrings.txt','Units\NightElfAbilityFunc.txt',
									'Units\NightElfAbilityStrings.txt','Units\NightElfUnitFunc.txt',
									'Units\NightElfUnitStrings.txt','Units\NightElfUpgradeFunc.txt',
									'Units\NightElfUpgradeStrings.txt','Units\OrcAbilityFunc.txt',
									'Units\OrcAbilityStrings.txt','Units\OrcUnitFunc.txt',
									'Units\OrcUnitStrings.txt','Units\OrcUpgradeFunc.txt',
									'Units\OrcUpgradeStrings.txt','Units\print.txt',
									'Units\UndeadAbilityFunc.txt','Units\UndeadAbilityStrings.txt',
									'Units\UndeadUnitFunc.txt','Units\UndeadUnitStrings.txt',
									'Units\UndeadUpgradeFunc.txt','Units\UndeadUpgradeStrings.txt');
			
		
		$mpq = new MPQArchive($mapfile, /*debug=*/false);

		if ($mpq)
		{
			$iniarray = array( );
			
			foreach ($objtxtdatafiles as $objtxtfile) 
			{
				$filedata = $mpq->readFile($objtxtfile);
				if ($filedata)
				{
					$iniarray = array_merge_recursive(self::parse_ini_string_m($filedata),$iniarray);
				}
			}
			if (!file_exists($mapicondir."/".$mapcode."/Icons"))
				mkdir( $mapicondir."/".$mapcode."/Icons",0777,true);
			self::write_ini_file($iniarray, $mapicondir."/".$mapcode."/objects.txt",true);
		}
		
		$objectids = array_keys($iniarray);
		
		foreach ($objectids as $objid) 
		{
			print("Obj:".$objid);
			if (array_key_exists('Art',$iniarray[$objid]))
				$art = explode( ',', $iniarray[$objid]['Art'])[0];
			
			if (array_key_exists('Name',$iniarray[$objid]))
				$objname = $iniarray[$objid]['Name'];
			if (!isset($objname))
			{
				if (array_key_exists('EditorName',$iniarray[$objid]))
					$objname = $iniarray[$objid]['EditorName'];
			}
			if (!isset($objname))
			{
				if (array_key_exists('Bufftip',$iniarray[$objid]))
					$objname = $iniarray[$objid]['Bufftip'];
			}
			if (!isset($objname))
			{
				if (array_key_exists('Bufftip',$iniarray[$objid]))
					$objname = $iniarray[$objid]['Bufftip'];
			}
			
			if (isset($objname))
			{
				file_put_contents($mapicondir."/".$mapcode."/Icons/".$objid.".txt",$objname);
			}
			
			if (isset($art) && strlen($art) > 0 )
			{
				if (self::ExtractIconToPath( $mpq, $art, $mapicondir, $mapicondir."/".$mapcode."/Icons/".$objid.".jpg"))
				{
					continue;
				}
			}
			else 
			{
				unset($art);
				if (array_key_exists('Unart',$iniarray[$objid]))
					$art =  explode( ',', $iniarray[$objid]['Unart'])[0];
				
				if (isset($art) && strlen($art) > 0 )
				{
					if (self::ExtractIconToPath( $mpq, $art, $mapicondir, $mapicondir."/".$mapcode."/Icons/".$objid.".jpg"))
					{
						continue;
					}
				}
				else  
				{
					unset($art);
					if (array_key_exists('Researchart',$iniarray[$objid]))
						$art =  explode( ',', $iniarray[$objid]['Researchart'])[0];
					
						
					if (isset($art) && strlen($art) > 0 )
					{
						if (self::ExtractIconToPath( $mpq, $art, $mapicondir, $mapicondir."/".$mapcode."/Icons/".$objid.".jpg"))
						{
							continue;
						}
					}
				}
			}
			
		
			
		}
		 
		 
		$mpq->close( );
		
		
	}
	
}