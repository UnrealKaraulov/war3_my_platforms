<?php

namespace Warcis\ServerNickName\Service\User;
use \Warcis\ServerNickName\PvPGNBot;

class EmailConfirmation extends XFCP_EmailConfirmation
{
	protected function advanceUserState()
	{
		$user = $this->user;
		$olduserstate = $user->user_state;
		parent::advanceUserState();	
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