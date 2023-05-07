<?php

namespace Warcis\ServerNickName;


// Bot login for PvPGN example
// (c) 2009,2015 HarpyWar

// set script execution timeout
set_time_limit(7);

// exec when execution time exceed or exception occured
//function handle_timeout()
//{ 
//    $error = error_get_last();
//
//    if ($error['type'] === E_ERROR) {
//		// do something to handle it
//		echo "error catched!";
 //   }
//}

//register_shutdown_function('handle_timeout');



class PvPGNBot 
{
	private $fp ;
	public $stream;
	public $resultstring = '';
	private $patterns=array(
		'*"*',
		'/*\n/'
	);

	// sequence of chat commands to be executed after login
	public $commands = array(
		"/exit"
	);
	
	public function bnbot2array($raw_protocol) {
        $output = array();
        $linenum = 0;
        foreach ($raw_protocol as $line) {
            $inquotes = false;
            $stage = 0;
            $line = trim($line);
            if ($line == "") continue;
            for ($x=0;$x<strlen($line);$x++) {
                $strip = false;
                if ($stage == 4 && $output[$linenum][0] == '1005') {
                }
                if ($line{$x} == ' ' && !$inquotes) {
                    $stage++;
                } elseif ($line{$x} == '"') {
                    if ($stage == 4 && $output[$linenum][0] == '1005') {
                        $strip = 4;
                        $inquotes = true;
                        if (isset($output[$linenum][$stage])) {
                            $output[$linenum][$stage] .= $line{$x};
                        } else {
                            $output[$linenum][$stage] = $line{$x};
                        }
                    } elseif ($stage == 2 && strstr($output[$linenum][0],'101')) {
                        $strip = 2;
                        $inquotes = true;
                        if (isset($output[$linenum][$stage])) {
                            $output[$linenum][$stage] .= $line{$x};
                        } else {
                            $output[$linenum][$stage] = $line{$x};
                        }
                    } else {
                        if ($inquotes)
                            $inquotes = false;
                        else
                            $inquotes = true;
                    }
                } else {
                    if (isset($output[$linenum][$stage])) {
                        $output[$linenum][$stage] .= $line{$x};
                    } else {
                        $output[$linenum][$stage] = $line{$x};                
                    }
                }
            }
            if ($strip) {
                $output[$linenum][$strip] = substr($output[$linenum][$strip],1,-1);
            }
            $linenum++;
        }
        return $output;
    }

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
			$data = trim( fgets($this->fp,4096 ) );
			if ( !empty($data) )
			{
				$this->stream[] = $data;
			}
		}
		//print_r($this->stream); # print buffer output
		 # print buffer output
	}

	public function SendCommand( $command )
	{
		$server="109.248.168.67";

		$port="6112";
		$login='WarcisCmdBot';
		$password='bottest2bottest';
		
		$this->commands = array(
			"/join The Void",
			$command,
			"/exit"
		);

		$this->Start($server,$port,$login,$password);
		return $this->bnbot2array($this->stream);
	}
}

