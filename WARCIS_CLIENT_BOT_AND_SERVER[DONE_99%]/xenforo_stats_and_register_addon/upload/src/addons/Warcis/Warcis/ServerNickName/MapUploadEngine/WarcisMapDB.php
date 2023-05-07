<?php

namespace Warcis\ServerNickName\MapUploadEngine;
use XF\Db\Schema\Alter;
use XF\Db\Schema\Create;


class WarcisMapDB 
{
	//public static function findMapByUserId( $userid )
	//{
	//	$db = \XF::db();
	//	$map = $db->fetchRow("SELECT * FROM warcis_maplist WHERE user_id = ?",$userid);
	//	return $map;
	//}
	
	public static function findMapsByUserId( $userid )
	{
		$db = \XF::db();
		$map = $db->fetchAll("SELECT * FROM warcis_maplist WHERE user_id = ?",[$userid]);
		return $map;
	}
	
	public static function findAllMaps( )
	{
		$db = \XF::db();
		$map = $db->fetchAll("SELECT * FROM warcis_maplist");
		return $map;
	}
	
	public static function findAllMapsByParam( $param, $val )
	{
		$db = \XF::db();
		$map = $db->fetchAll("SELECT * FROM warcis_maplist WHERE ? = ?",[$param,$val]);
		return $map;
	}
	
	public static function getMapsCount( )
	{
		$db = \XF::db();
		$map = $db->fetchOne("SELECT COUNT(id) FROM warcis_maplist");
		return $map;
	}
	
	public static function getMapsFromRange( $xstart, $xcount )
	{
		$db = \XF::db();
		$map = $db->fetchAll("SELECT * FROM warcis_maplist LIMIT ?, ?",[$xstart , $xcount]);
		return $map;
	}
	
	public static function getMapsFromRangeByParam( $param, $val, $xstart, $xcount  )
	{
		$db = \XF::db();
		$map = $db->fetchAll("SELECT * FROM warcis_maplist WHERE $param = ? LIMIT ?, ? ",[ $val, $xstart , $xcount ]);
		return $map;
	}
	
	public static function getMapsCountByParam( $param, $val )
	{
		$db = \XF::db();
		$map = $db->fetchOne("SELECT COUNT(id) FROM warcis_maplist WHERE $param = ?",[$val]);
		return $map;
	}
	
	public static function findMapById( $mapid )
	{
		$db = \XF::db();
		$map = $db->fetchRow("SELECT * FROM warcis_maplist WHERE id = ?",[$mapid]);
		return $map;
	}

	public static function findMapByHostCmd( $hostcmd )
	{
		$db = \XF::db();
		$map = $db->fetchRow("SELECT * FROM warcis_maplist WHERE hostcmd = ?", [$hostcmd]);
		return $map;
	}

	public static function findMapByCrc32( $mapcrc32 )
	{
		$db = \XF::db();
		$map = $db->fetchRow("SELECT * FROM warcis_maplist WHERE crc32 = ?", [$mapcrc32]);
		return $map;
	}

	public static function deleteMapById( $mapid )
	{
		$db = \XF::db();
		$db->query("DELETE FROM warcis_maplist WHERE id = ?", [$mapid]);
	}

	public static function addNewMap( $hostcmd, $name, $description, $userdescription, $category, $author, $user_id, $username, $stats, $filename, $mapcrc32)
	{
		$db = \XF::db();
		$map = $db->query(
		"
		INSERT INTO warcis_maplist
		(id, hostcmd, name, description, userdescription, category, author, user_id, username, downloads, rating, stats, filename, crc32, activated)
		VALUES 
		(NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
		", [$hostcmd,$name,$description,$userdescription,$category,$author,$user_id, $username, 0,0,$stats ? 1 : 0,$filename,$mapcrc32,0]);
		return $map;
	}

	public static function updateMapById($mapid, $hostcmd, $name, $description, $category, $author, $downloads, $rating, $stats, $filename)
	{
		$db = \XF::db();
		$db->query(
		"
		UPDATE warcis_maplist 
		SET 
		hostcmd = ?,
		name = ?,
		description = ?,
		userdescription = ?,
		category = ?,
		author = ?,
		downloads = ?,
		rating = ?,
		stats = ?,
		filename = ?
		WHERE id = ?
		", [$hostcmd,$name,$description,$category,$author,$downloads, $rating,$stats ? 1 : 0,$filename,$mapid]);
	}

	public static function updateMapParamById($mapid, $paramname,$paramvalue)
	{
		$db = \XF::db();
		$db->query(
		"
		UPDATE warcis_maplist 
		SET 
		? = ?
		WHERE id = ?
		", [$paramname,$paramvalue,$mapid]);
	}

	
}
