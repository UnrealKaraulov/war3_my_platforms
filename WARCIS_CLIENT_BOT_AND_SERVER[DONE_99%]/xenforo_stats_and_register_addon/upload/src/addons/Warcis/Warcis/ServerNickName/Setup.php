<?php

namespace Warcis\ServerNickName;

use XF\AddOn\AbstractSetup;
use XF\Db\Schema\Alter;
use XF\Db\Schema\Create;


class Setup extends \XF\AddOn\AbstractSetup
{

	public function install(array $stepParams = [])
    {
	
        $this->schemaManager()->alterTable('xf_user', function(Alter $table)
        {
            $table->addColumn('nickname', 'varchar', 100)->setDefault('');
        });
		
		$this->schemaManager()->alterTable('wc_bnet', function(Alter $table)
        {
			$table->addColumn('acct_username_forum', 'varchar', 128)->setDefault('');
			$table->addColumn('acct_playingmap', 'varchar', 100)->setDefault('');
			$table->addColumn('acct_st_dota88_mmr', 'INT')->setDefault(1000);
			$table->addColumn('acct_st_dota_mmr', 'INT')->setDefault(1000);
			$table->addColumn('acct_st_dotalod_mmr', 'INT')->setDefault(1000);
			$table->addColumn('acct_eventid_1', 'INT')->setDefault(0);
			$table->addColumn('acct_eventid_2', 'INT')->setDefault(0);
			$table->addColumn('acct_eventid_3', 'INT')->setDefault(0);
			$table->addColumn('acct_eventid_4', 'INT')->setDefault(0);
			$table->addColumn('acct_eventid_5', 'INT')->setDefault(0);
			$table->addColumn('acct_eventid_6', 'INT')->setDefault(0);
			$table->addColumn('acct_eventid_7', 'INT')->setDefault(0);
			$table->addColumn('acct_eventid_8', 'INT')->setDefault(0);
			$table->addColumn('acct_eventid_9', 'INT')->setDefault(0);
			$table->addColumn('acct_eventid_10', 'INT')->setDefault(0);
			$table->addColumn('acct_eventid_11', 'INT')->setDefault(0);
        });
		
		$this->schemaManager()->createTable('warcis_maplist', function(Create $table)
        {
            $table->addColumn('id', 'INT')->primaryKey()->autoIncrement();
			$table->addColumn('hostcmd', 'varchar', 50);
			$table->addColumn('name', 'varchar', 256 );
			$table->addColumn('description', 'varchar', 1024 )->setDefault("no description");
			$table->addColumn('userdescription', 'varchar', 1024 )->setDefault("no description");
			$table->addColumn('category', 'varchar', 128 );
			$table->addColumn('author', 'varchar', 128 )->setDefault("Blizzard or unknown");
			$table->addColumn('user_id', 'INT' );
			$table->addColumn('username', 'varchar', 256 );
			$table->addColumn('downloads', 'INT' )->setDefault(0);
			$table->addColumn('rating', 'INT' )->setDefault(0);
			$table->addColumn('stats', 'INT' )->setDefault(0);
			$table->addColumn('filename', 'varchar', 128 );
			$table->addColumn('crc32', 'INT' )->setDefault(0);
			$table->addColumn('activated', 'INT' )->setDefault(0);
        });
		
    }
	
	
	public function upgrade(array $stepParams = [])
    {
        
    }
	
	
    public function uninstall(array $stepParams = [])
    {
		$this->schemaManager()->alterTable('xf_user', function(Alter $table)
        {
			$columnstodrop = [ 'nickname', ];
            $table->dropColumns($columnstodrop);
        });
		
		$this->schemaManager()->dropTable('warcis_maplist');
    }

}
