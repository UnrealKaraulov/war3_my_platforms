<?php

namespace Warcis\ServerNickName\WarcraftStatsEngine;
use XF\Db\Schema\Alter;
use XF\Db\Schema\Create;


class WarcraftStats 
{
	public static function ClearWc3Tables( )
	{
		
		$db = \XF::db();
		$db->emptyTable("wc_BNET",[$nickname]);
		$db->emptyTable("wc_clan",[$nickname]);
		$db->emptyTable("wc_clanmember",[$nickname]);
		$db->emptyTable("wc_friend",[$nickname]);
		$db->emptyTable("wc_profile",[$nickname]);
		
		
		return $waracc;
	}
	
	
	public static function GetWarcraftAccountByName( $nickname )
	{
		//print("Get account by nickname: $nickname");
		$db = \XF::db();
		$waracc = $db->fetchRow("SELECT * FROM wc_BNET WHERE acct_username = ?",[$nickname]);
		return $waracc;
	}
	
	public static function GetWarcraftAccountsByMapCode( $mapcode )
	{
		//print("Get account by nickname: $nickname");
		$db = \XF::db();
		$waracc = $db->fetchAll("SELECT acct_username, acct_username_forum, acct_playingmap FROM wc_BNET WHERE acct_playingmap = ?",[$mapcode]);
		return $waracc;
	}
	
	public static function GetWarcraftTopPlayersByStateBAD( $statscode, $maxcount )
	{
		//print("Get account by nickname: $nickname");
		$db = \XF::db();
		$waracc = $db->fetchAll("SELECT acct_username, acct_username_forum, ? FROM wc_BNET WHERE ? AND acct_username_forum <> '' ORDER BY ? DESC LIMIT ?",[$statscode,$statscode,$statscode, $maxcount]);
		return $waracc;
	}
	
	
	
	public static function GetWarcraftTopPlayersByState( $statscode, $maxcount )
	{
		//print("Get account by nickname: $nickname");
		$db = \XF::db();
		$waracc = $db->fetchAll("SELECT acct_username, acct_username_forum, $statscode FROM wc_BNET WHERE $statscode AND acct_username_forum <> '' ORDER BY $statscode DESC LIMIT ?",[ $maxcount]);
		return $waracc;
	}
	
	public static function GetWarcraftAccountsWithMapCode( )
	{
		//print("Get account by nickname: $nickname");
		$db = \XF::db();
		$waracc = $db->fetchAll(" 
		
		SELECT acct_username, acct_username_forum, acct_playingmap FROM wc_BNET WHERE acct_playingmap IN 
		(
		    SELECT * FROM 
			(
				SELECT DISTINCT acct_playingmap FROM wc_BNET WHERE acct_playingmap <> '' LIMIT 4 
			) AS FUUK_MYSQL
		)  
		
		");
		return $waracc;
	}
	
	public static function GetWarcraftAccountsWithMapCodeLimited( $startid, $count )
	{
		//print("Get account by nickname: $nickname");
		$db = \XF::db();
		$waracc = $db->fetchAll("SELECT acct_username, acct_username_forum, acct_playingmap FROM wc_BNET WHERE acct_playingmap <> ''");
		return $waracc;
	}
	
	public static function GetWarcraftAccountsByMapCodeFULL( $mapcode )
	{
		//print("Get account by nickname: $nickname");
		$db = \XF::db();
		$waracc = $db->fetchAll("SELECT * FROM wc_BNET WHERE acct_playingmap = ?",[$mapcode]);
		return $waracc;
	}
	
	public static function GetWarcraftAccountsWithMapCodeFULL(  )
	{
		//print("Get account by nickname: $nickname");
		$db = \XF::db();
		$waracc = $db->fetchAll("SELECT * FROM wc_BNET WHERE acct_playingmap <> ''");
		return $waracc;
	}
	
	public static function GetWarcraftStatsByMapCode( $warcraftacc, $mapcode, $statstype, $isinteger = true )
	{
		$statsline = "acct_st_".$mapcode."_".$statstype;
		
		if (!empty($warcraftacc) && is_array($warcraftacc) && in_array($statsline,$warcraftacc))
			return $isinteger ? intval( $warcraftacc[ $statsline ] ) : $warcraftacc[ $statsline ] ;
		return $isinteger ? 0 : "";
	}
	
	public static function GetWarcraftStats( $warcraftacc, $statstype, $isinteger = true )
	{
		if (!empty($warcraftacc) && is_array($warcraftacc) && in_array($statstype,$warcraftacc))
			return $isinteger ? intval( $warcraftacc[ $statstype ]): $warcraftacc[ $statstype ];
		return $isinteger ? 0 : "";
	}
}
