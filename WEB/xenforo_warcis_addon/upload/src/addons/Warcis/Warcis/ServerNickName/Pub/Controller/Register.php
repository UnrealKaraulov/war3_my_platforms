<?php

namespace Warcis\ServerNickName\Pub\Controller;

use \XF\Mvc\ParameterBag;
use \XF\Mvc\Reply\View;
use \XF\Mvc\FormAction;
use \XF\Entity\User;
use \Warcis\ServerNickName\PvPGNBot;

class Register extends XFCP_Register
{
	public function actionRegister()
	{
		$nickname = trim($_POST["nickname"]);
		if (strlen($nickname) > 2 && strlen($nickname) < 16)
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
				return $this->error(\XF::phrase('check_acc_error').$error);
			}
	
			
			// get parent		
			$parent = parent::actionRegister();	

			// get visitor
			$visitor = \XF::visitor();			

			// get userId
			$userId = $visitor['user_id'];	
			$username = $visitor['username'];		
			$email = $visitor['email'];				
			if ($userId > 0)
			{
				if ($visitor->Auth && $visitor->Auth->data)
				{
				
					//echo("<div>Hello,".$userId."</div>"."<div>Hello2,".$nickname."</div>");
				
							
					$error = 'Error.';
					$pvpgnBot_my = new PvPGNBot( );
					$userauthstr = $visitor->Auth->data['hash'];
					$userauthstr[2] = 'a';
					$result = $pvpgnBot_my->SendCommand( "/register_warcis_acc"." ".$nickname." ".$userauthstr." ".$username." ".$email );
					
					
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
						return $this->error(\XF::phrase('check_acc_error').$error);
					}
					
					$visitor['nickname'] = $nickname;
					$visitor->save(); 
			
					return $parent;	
				}
				else 
				{
					return $this->error(\XF::phrase('check_acc_error').$error);
				}
				//$user = $this->finder('XF:User')->where('user_id', $userId)->fetchOne();
				

			}
			// return parent
			return $parent;		
		}
		else 
		{
			return $this->error(\XF::phrase('bad_nickname'));
		}
	}
}