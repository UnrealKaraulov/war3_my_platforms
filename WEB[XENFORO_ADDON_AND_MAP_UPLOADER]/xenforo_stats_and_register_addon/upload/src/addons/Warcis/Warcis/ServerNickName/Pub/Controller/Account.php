<?php

namespace Warcis\ServerNickName\Pub\Controller;

use \XF\Mvc\ParameterBag;
use \XF\Mvc\Reply\View;
use \XF\Mvc\FormAction;
use \XF\Entity\User;
use \Warcis\ServerNickName\PvPGNBot;

class Account extends XFCP_Account
{
	public function actionVisitorMenu()
	{
		$parent = parent::actionVisitorMenu();	
		//$visitor = \XF::visitor(); 
		
		return $parent;
	}
	
	protected function accountDetailsSaveProcess(\XF\Entity\User $visitor)
	{	
		$oldnickname = $visitor['nickname'];
				
		$nickname = trim($this->filter([
			'nickname' => 'str'
		])['nickname']);

	
		$parent = parent::accountDetailsSaveProcess($visitor);	
		
	
		if ($nickname && strlen($nickname) > 2 && strlen($nickname) < 16)
		{
			$pvpgnBot_my = new PvPGNBot( );
			$result = $pvpgnBot_my->SendCommand( "/check_warcis_acc ".$nickname );
			$error = 'Error.';
			foreach ($result as $value)
			{
				if (is_array($value))
				{
					foreach ($value as $value2)
					{
						if (strstr($value2,"Invalid."))
						{
							$error = "Invalid";
							break;
						}
						else if (strstr($value2,"Success."))
						{
							$error = '';
							break;
						}
						else if (strstr($value2,"AlreadyRegistered."))
						{
							$error = 'AlreadyRegistered.';
							break;
						}
					}
				}
				else 
				{
					if (strstr($value,"Invalid."))
					{
						$error = "Invalid";
						break;
					}
					else if (strstr($value,"Success."))
					{
						$error = '';
						break;
					}
					else if (strstr($value,"AlreadyRegistered."))
					{
						$error = 'AlreadyRegistered.';
						break;
					}
				}
			}
			
			if ( $error != '' )
			{
				$parent->logError(\XF::phrase('bad_nickname')."[2]");
				return $parent;
			}
			
			
			if ( !$oldnickname || strlen($oldnickname) == 0 )
			{
				$userId = $visitor['user_id'];	
				$username = $visitor['username'];	
				$email = $visitor['email'];				
				if ($userId > 0)
				{
					if ($visitor->Auth && $visitor->Auth->data)
					{
						$error = 'Error.';
						$pvpgnBot_my = new PvPGNBot( );
						$userauthstr = $visitor->Auth->data['hash'];
						$userauthstr[2] = 'a';
						$result = $pvpgnBot_my->SendCommand( "/register_warcis_acc"." ".$nickname." ".$userauthstr." ".$username." ".$email);
						
						
						foreach ($result as $value)
						{
							if (is_array($value))
							{
								foreach ($value as $value2)
								{
									if (strstr($value2,"Invalid."))
									{
										$error = "Invalid";
										break;
									}
									else if (strstr($value2,"Success."))
									{
										$error = '';
										break;
									}
									else if (strstr($value2,"AlreadyRegistered."))
									{
										$error = 'AlreadyRegistered.';
										break;
									}
								}
							}
							else 
							{
								if (strstr($value,"Invalid."))
								{
									$error = "Invalid";
									break;
								}
								else if (strstr($value,"Success."))
								{
									$error = '';
									break;
								}
								else if (strstr($value,"AlreadyRegistered."))
								{
									$error = 'AlreadyRegistered.';
									break;
								}
							}
						}
						
						if ( $error != '' )
						{
							return $this->error(\XF::phrase('check_acc_error').$error."[1]");
						}
						
						
						$visitor['nickname'] = $nickname;
						$visitor->save(); 
					}
				}
			}
			return $parent;
		}
		
		return $parent;
	}
}