<?php

namespace Warcis\ServerNickName\Pub\Controller;

use XF\Mvc\ParameterBag;
use XF\Mvc\FormAction;
use XF\Mvc\Reply\View;
use XF\Pub\Controller\AbstractController;
use Warcis\ServerNickName\WarcraftStatsEngine\WarcraftStats;

class UserStats extends AbstractController
{
	public function actionIndex()
	{
		$visitor = \XF::visitor(); 
		$viewParams = [ ];
		
		$StatsMapsStruct = [];
		
		$usersplayingmap = WarcraftStats::GetWarcraftAccountsWithMapCode();
		foreach ($usersplayingmap as $playuser) 
		{
			
			// check array [mapcode] exists 
			if (!array_key_exists($playuser['acct_playingmap'], $StatsMapsStruct))
			{
				$StatsMapsStruct += array($playuser['acct_playingmap'] => array( $playuser['acct_username']));
			}
			else 
			{
				$StatsMapsStruct[$playuser['acct_playingmap']][] = $playuser['acct_username'];
			}
		}
		
		$viewParams = ['OutContext' => "Stats print..."];
		$viewParams += ['MAPS_STATS' => $StatsMapsStruct];
		$viewParams += ['TOP_MMR_DOTA88' => WarcraftStats::GetWarcraftTopPlayersByState('acct_st_dota88_mmr',10)];
		$viewParams += ['TOP_MMR_DOTA' => WarcraftStats::GetWarcraftTopPlayersByState('acct_st_dota_mmr',10)];
		$viewParams += ['TOP_MMR_DOTALOD' => WarcraftStats::GetWarcraftTopPlayersByState('acct_st_dotalod_mmr',10)];
		
		return $this->view('XF:UserStats\UserStats', 'userstats', $viewParams);
	}
	public function Index()
	{
		$visitor = \XF::visitor(); 
		$viewParams = [ ];
		if (!$visitor->user_id || !$visitor->username)
		{
			$viewParams = [
				'OutContext' => "You not registered",
			]; 
		}	
		else 
		{
			$viewParams = [
				'OutContext' => "Hello1,".$visitor->username,
			]; 
		}
		return $this->view('XF:UserStats\UserStats', 'userstats', $viewParams);
	}
}