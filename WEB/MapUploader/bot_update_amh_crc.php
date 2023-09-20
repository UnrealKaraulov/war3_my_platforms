<?php

// Bot login for PvPGN example
// (c) 2009,2015 HarpyWar

// set script execution timeout
set_time_limit(10);
register_shutdown_function('handle_timeout');

// exec when execution time exceed or exception occured
function handle_timeout()
{ 
    $error = error_get_last();

    if ($error['type'] === E_ERROR) {
		// do something to handle it
		echo "error catched!";
    }
}


class pvpgnBot 
{
	private $fp ;
	public $stream;
	public $resultstring = "Error";
	private $patterns=array(
		'*"*',
		'/*\n/'
	);

	// sequence of chat commands to be executed after login
	public $commands = array(
		"/join The Void",
		"/setahcrcvalue 3944752218",
		"/exit"
	);

	public function Start($server,$port,$login,$password)
	{
		$this->fp = @fsockopen($server, $port, $errno, $errstr, 5);
		if (!$this->fp) {
			die ("ERROR: Can't connect to $server:$port. $errno - $errstr"); //na?aa? auee??ai
		}
		// login to server
		$cmdline = "\x03\x04{$login}\v{$password}\x0d".password_hash($password,PASSWORD_BCRYPT)."\n";
		// append commands
		foreach ($this->commands as $cmd)
			$cmdline .= $cmd . "\n";

		$this->sendLine($cmdline);
		fclose($this->fp);
	}
	
	private function sendLine($str)
	{
		fwrite ($this->fp, $str); 
		while (!feof($this->fp))
		{
			$data = trim( fgets($this->fp, 4086) );
			if ( !empty($data) )
			{
				$this->stream[] = $data;

				$forprint = preg_replace('/(\d+) (\w+) (.*)/', '$3',  $data);
				if ($data != $forprint)
				{
					print(preg_replace('/\W/', ' ',  $forprint).'<div/>');
				}
			}
		}
		 # print buffer output
	}

	public function Init( )
	{
		// ENTER HERE PVPGN SERVER IP!
		$server="127.0.0.1";

		$port="6112";
		$login='WarcisCmdBot';
		$password='bottest2bottest';

		$bot = $this->Start($server,$port,$login,$password);
		
		$resultstring = "All Okay";
		echo "<b>$server</b><br><br>";
	}
}

