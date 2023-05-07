<?php

namespace Warcis\ServerNickName\Admin\Controller;

use XF\Mvc\FormAction;
use XF\Mvc\ParameterBag;

use \Warcis\ServerNickName\PvPGNBot;

class User extends XFCP_User
{
	public function actionSave(ParameterBag $params)
	{
		$user = null;
		$olduserstate = 'valid';
		if ($params->user_id)
		{
			$user = $this->assertUserExists($params->user_id);
			$this->assertCanEditUser($user);
			$user = $this->getUserRepo()->setupBaseUser($user);
			$olduserstate = $user->user_state;
		}
	
		$parent = parent::actionSave( $params);
	
		if ($params->user_id)
		{
			$user = $this->assertUserExists($params->user_id);
			$this->assertCanEditUser($user);
			$user = $this->getUserRepo()->setupBaseUser($user);
		}
		else 
		{
			$user = null;
		}
	
		if ($user != null && $olduserstate != $user->user_state && $user->user_state == 'valid')
		{
			$pvpgnBot_my = new PvPGNBot( );
			$result = $pvpgnBot_my->SendCommand( "/activate_warcis_acc ".$user->nickname );
		}
		
		return $parent;
	}

}