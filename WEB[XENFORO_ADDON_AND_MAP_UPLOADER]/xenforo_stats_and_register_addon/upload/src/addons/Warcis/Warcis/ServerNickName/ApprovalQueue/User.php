<?php

namespace Warcis\ServerNickName\ApprovalQueue;

use XF\Entity\ApprovalQueue;
use XF\Mvc\Entity\Entity;
use \Warcis\ServerNickName\PvPGNBot;

class User extends XFCP_User
{
	public function actionApprove(\XF\Entity\User $user)
	{
		$olduserstate = $user->user_state;
		parent::actionApprove($user);	
		if ($olduserstate != $user->user_state && $user->user_state == 'valid')
		{
			$pvpgnBot_my = new PvPGNBot( );
			$result = $pvpgnBot_my->SendCommand( "/activate_warcis_acc ".$user->nickname );
			$error = 'Error.';
			foreach ($result as $value)
			{
				if (is_array($value))
				{
					foreach ($value as $value2)
					{
						if (strstr($value2,"Success."))
						{
							$error = '';
							break;
						}
					}
				}
				else 
				{
					if (strstr($value,"Success."))
					{
						$error = '';
						break;
					}
				}
			}
			if ($error != '')
				$error = \XF::phrase('check_acc_error').$error;
		}
	}
}