<?php

namespace Warcis\ServerNickName\Pub\Entity;
use XF\Mvc\Entity\Structure;

class User extends XFCP_User {
	public static function getStructure(Structure $structure) 
	{
		$structure = parent::getStructure($structure);	
		
		$structure->columns = array_merge(
		$structure->columns, 
		['nickname' => ['type' => self::STR, 'maxLength' => 50, 'default' => ''	]]
		);
		
		return 	$structure;
	}
}