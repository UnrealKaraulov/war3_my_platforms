<?php

namespace Warcis\ServerNickName\Admin\Controller;

use XF\Entity\OptionGroup;
use XF\Mvc\FormAction;
use XF\Mvc\ParameterBag;


class Option extends XFCP_Option
{
	public function actionWarcis(ParameterBag $params)
	{
		return $this->view('XF:Option\Warcis', 'warcis_options', []);
	}

}