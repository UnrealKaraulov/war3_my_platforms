<?php

namespace Warcis\ServerNickName\Pub\Controller;

use XF\Entity\User;
use XF\Entity\UserProfile;
use XF\Mvc\FormAction;
use XF\Mvc\ParameterBag;
use Warcis\ServerNickName\PvPGNBot;
use Warcis\ServerNickName\WarcraftStatsEngine\WarcraftStats;
use Warcis\ServerNickName\MapUploadEngine\WarcisMapDB;
use XF\Db\Schema\Alter;
use XF\Db\Schema\Create;

class Member extends XFCP_Member
{
	public function GetWarStats( $nickname )
	{
		if (empty($nickname) || strlen($nickname) < 2 || strlen($nickname) > 15)
		{
			return "<div>error</div>";
		}
		
		$war3acc = WarcraftStats::GetWarcraftAccountByName($nickname);
		$acctlocked = WarcraftStats::GetWarcraftStats($war3acc,"auth_lock");
		$acctmuted = WarcraftStats::GetWarcraftStats($war3acc,"auth_mute");
		
		$sendcommands = WarcraftStats::GetWarcraftStats($war3acc,"acct_eventid_1");
		$sendchannelmessages = WarcraftStats::GetWarcraftStats($war3acc,"acct_eventid_2");
		$sendprivatemessages = WarcraftStats::GetWarcraftStats($war3acc,"acct_eventid_3");
		$recvprivatemessages = WarcraftStats::GetWarcraftStats($war3acc,"acct_eventid_4");
		$minutesinwarcraft = WarcraftStats::GetWarcraftStats($war3acc,"acct_eventid_5");
		$hostedgames = WarcraftStats::GetWarcraftStats($war3acc,"acct_eventid_6");
		$minutesingame = WarcraftStats::GetWarcraftStats($war3acc,"acct_eventid_7");
		$acctbans = WarcraftStats::GetWarcraftStats($war3acc,"acct_eventid_8");
		$playingmap = WarcraftStats::GetWarcraftStats($war3acc,"acct_playingmap");
		
		$mapanotherinfo = WarcisMapDB::findMapByHostCmd($playingmap);
		if ($mapanotherinfo)
		{
			$playingmap = $mapanotherinfo['name'] . "  (".$mapanotherinfo['filename'].")";
		}
		
		$totaltimeatserver = $minutesingame + $minutesinwarcraft;
		
		return [
			//"war3acc" => $war3acc,
			"acctlocked" => $acctlocked,
			"acctmuted" => $acctmuted,
			
			"sendcommands" => $sendcommands,
			"sendchannelmessages" => $sendchannelmessages,
			"sendprivatemessages" => $sendprivatemessages,
			"recvprivatemessages" => $recvprivatemessages,
			"minutesingame" => $minutesingame,
			"hostedgames" => $hostedgames,
			"minutesinwarcraft" => $minutesinwarcraft,
			"totaltimeatserver" => $totaltimeatserver,
			"acctbans" => $acctbans,
			"playingmap" => $playingmap
		];
	}
	
	public function actionWarcisantihackcrc(ParameterBag $params)
	{
		$visitor = \XF::visitor();
		if (!$visitor || !$visitor->is_admin)
		{
			$error = "You not have moderate privileges.";
			print($error);
			return $this->view('XF:Member\RecreateAllAccounts', '', ["error"=>$error]);
		}
		$viewParams = [ 
			"OutContext" => "NO"
		];
		if( isset($_POST["crc32"]) ) 
		{
			$viewParams = [ 
				"OutContext" => $_POST["crc32"]
			];
			$pvpgnBot_my = new PvPGNBot( );
			$pvpgnBot_my->SendCommand( "/setahcrcvalue ".$_POST["crc32"] );
		}
		
		
		return $this->view('XF:Member\Warcisantihackcrc', 'amh_crc_update', $viewParams);
	}
	
	
	public function actionRenamenickname(ParameterBag $params)
	{	
		$visitor = \XF::visitor();
		if (!$visitor || !$visitor->is_admin)
		{
			$error = "You not have moderate privileges.";
			print($error);
			return $this->view('XF:Member\RecreateAllAccounts', '', ["error"=>$error]);
		}
		
		$viewParams = [ 
			"OutContext" => "Sorry rename disabled."
		];
		
		return $this->view('XF:Member\Warcraft', 'amh_crc_update', $viewParams);
	}
	
	
	public function actionWarcraft(ParameterBag $params)
	{
		$user = $this->assertViewableUser($params->user_id);
		
		$viewParams = [ 
			"username" => $user->username,
			"nickname" => $user->nickname
		];
		
		$userstats = $this->GetWarStats($user->nickname);
		
		$viewParams = array_merge(	$viewParams,$userstats);
		
		return $this->view('XF:Member\Warcraft', 'warcraft_simple_stats', $viewParams);
	}
	
	public static function recreateBotAccount1( )
	{
		$db = \XF::db();
		$map = $db->query(
		"
		INSERT INTO wc_bnet 
		(uid, acct_username, username, username_forum, acct_userid, acct_passhash1, acct_email, auth_admin, auth_normallogin, auth_changepass, auth_changeprofile, auth_botlogin, auth_operator, new_at_team_flag, auth_lock, auth_locktime, auth_lockreason, auth_mute, auth_mutetime, auth_mutereason, auth_command_groups, acct_lastlogin_time, acct_lastlogin_owner, acct_lastlogin_clienttag, acct_lastlogin_ip) 
		VALUES
		(?, ?, ?, ?, ?, ?, ?, false, true, true, true, true, false, 0, ?, 0, ?, 0, 0, NULL, ?, 0, NULL, NULL, NULL),
		", [1,"WarcisCmdBot", "warciscmdbot", "warciscmdbot",1,"$2a$10$O0bgQ03CUCXkcjLSYjHuPuRXyKUpLCymOn0R2.AKF8RPlCtPZgW16","",0,"", 255]);
		return $map;
	}
	
	public static function recreateBotAccount2( )
	{
		$db = \XF::db();
		$map = $db->query(
		"
		INSERT INTO wc_bnet 
		(uid, acct_username, username, username_forum, acct_userid, acct_passhash1, acct_email, auth_admin, auth_normallogin, auth_changepass, auth_changeprofile, auth_botlogin, auth_operator, new_at_team_flag, auth_lock, auth_locktime, auth_lockreason, auth_mute, auth_mutetime, auth_mutereason, auth_command_groups, acct_lastlogin_time, acct_lastlogin_owner, acct_lastlogin_clienttag, acct_lastlogin_ip) 
		VALUES
		(?, ?, ?, ?, ?, ?, ?, false, true, true, true, false, false, 0, ?, 0, ?, 0, 0, NULL, ?, 0, NULL, NULL, NULL),
		", [2,"WarcisHostBot", "warcishostbot", "warcishostbot",2,"$2a$10$ZS3rY1LVSi3JOyXSSzjxWO2Pxl/DVGWPQLwS.0kLW4NpMVko6Q9FS","",0,"", 255]);
		return $map;
	}
	
	
		
	public static function clearPVPGN_TABLES( )
	{
		$db = \XF::db();
		$map = $db->query(
		"
			TRUNCATE TABLE wc_arrangedteam
			TRUNCATE TABLE wc_bnet
			TRUNCATE TABLE wc_clan
			TRUNCATE TABLE wc_clanmember
			TRUNCATE TABLE wc_friend
			TRUNCATE TABLE wc_profile
		");
		return $map;
	}
	
	
	
	public static function addNewAccount( $uid, $username, $username_to_lower, $forum_name, $accpassword, $accemail, $accactivated, $lockreason )
	{
		$db = \XF::db();
		$map = $db->query(
		"
		INSERT INTO wc_bnet 
		(uid, acct_username, username, username_forum, acct_userid, acct_passhash1, acct_email, auth_admin, auth_normallogin, auth_changepass, auth_changeprofile, auth_botlogin, auth_operator, new_at_team_flag, auth_lock, auth_locktime, auth_lockreason, auth_mute, auth_mutetime, auth_mutereason, auth_command_groups, acct_lastlogin_time, acct_lastlogin_owner, acct_lastlogin_clienttag, acct_lastlogin_ip) 
		VALUES
		(?, ?, ?, ?, ?, ?, ?, false, true, true, true, false, false, 0, ?, 0, ?, 0, 0, NULL, ?, 0, NULL, NULL, NULL),
		", [$uid,$username,$username_to_lower,$forum_name,$uid,$accpassword,$accemail,$accactivated,$lockreason, 1]);
		return $map;
	}
	
	public function actionRecreateallaccounts(ParameterBag $params)
	{
		$visitor = \XF::visitor();
		if (!$visitor || !$visitor->is_admin)
		{
			$error = "You not have moderate privileges.";
			print($error);
			return $this->view('XF:Member\RecreateAllAccounts', '', ["error"=>$error]);
		}
		
		$db = \XF::db();
		$users = $db->fetchAll("SELECT user_id FROM xf_user");
	
		$commandstosend  = "";
		$pvpgnBot_my = new PvPGNBot( );
				
				
		$this->clearPVPGN_TABLES( );
						
		$this->recreateBotAccount1( );
		$this->recreateBotAccount2( );
		
		$cuserid = 2;		
				
		foreach ($users as $userid)
		{	
			$cuserid = $cuserid + 1;
			$user = $this->finder('XF:User')->where('user_id', $userid)->fetchOne();
			if (!$user)
			{
				print("<div>.</div> No user!");
			}
			else if (!$user->nickname || strlen($user->nickname) < 2)
			{
				print("<div>.</div> No nickname!");
			}
			else 
			{
				$nickname = $user->nickname;
				$username = $user->username;
				$email = $user->email;
				print("<div>.</div> Try to create account for:".$user->nickname.". Forum username:".$user->username);
				//$commandstosend.= "/check_warcis_acc ".$user->nickname."\r\n";
				/*$error = 'Error.';
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
				
				if ( $error != '' && $error != 'AlreadyRegistered.')
				{
					print("<div>.</div> Error check account:".$error);
				}
				else 
				{*/
				
					if ($user->Auth && $user->Auth->data)
					{
						//$pvpgnBot_my = new PvPGNBot( );
						$userauthstr = $user->Auth->data['hash'];
						$userauthstr[2] = 'a';
						
						$accountactivatestate = ($user->user_state == 'valid') ? "0" : "1";
						$accountlockreason = ($user->user_state == 'valid') ? "" : "Need confirm email!";
						
						
						$this->addNewAccount($cuserid,$nickname,strtolower($nickname),$username,$userauthstr,$email,$accountactivatestate,$accountlockreason);
						
						
						
						/*
						
						
						
						
						
						
						*/
						
						
						//print("<div>.</div> Reset user info...");
					
						//$commandstosend.= "/register_warcis_acc $nickname $userauthstr $username"."\r\n";
						
						//$commandstosend.= "/set_warcis_acc $nickname $userauthstr $username" ."\r\n";
					
						/*$error = 'Error.';
						
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
						
						
						if ( $error != '' && $error != 'AlreadyRegistered.')
						{
							print("<div>.</div> Error register account:".$error);
							continue;
						}
						else 
						{
							print("<div>.</div> Success! Account registered!");
						}
					
						*/
						//if($user->user_state == 'valid')
						//{
						//	print("<div>.</div> Activate account...");
							//$pvpgnBot_my = new PvPGNBot( );
						//	$commandstosend.= "/activate_warcis_acc $nickname"."\r\n";
							/*$error = 'Error.';
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
							{
									print("<div>.</div> Error activate account:".$error);
							}*/
						//}
						//else 
						//{
						//	print("<div>.</div> User need to confirm email for activate nickname at Warcis");
						//	
						//}
					}
				//}
			}
		}
		//$result = $pvpgnBot_my->SendCommand( $commandstosend );
		$result = $pvpgnBot_my->SendCommand( "/restart" );
		
		
		return $this->view('XF:Member\RecreateAllAccounts', '', []);
	}
	
	
}