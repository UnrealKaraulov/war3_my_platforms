-- phpMyAdmin SQL Dump
-- version 4.7.3
-- https://www.phpmyadmin.net/
--
-- Хост: 127.0.0.1:3306
-- Время создания: Ноя 05 2017 г., 17:58
-- Версия сервера: 10.1.25-MariaDB
-- Версия PHP: 7.1.7

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- База данных: `pvpgn1`
--

-- --------------------------------------------------------

--
-- Структура таблицы `gneve1_arrangedteam`
--

CREATE TABLE `gneve1_arrangedteam` (
  `teamid` int(11) NOT NULL DEFAULT '0',
  `size` int(11) DEFAULT '0',
  `clienttag` varchar(8) DEFAULT NULL,
  `lastgame` int(11) DEFAULT '0',
  `member1` int(11) DEFAULT '0',
  `member2` int(11) DEFAULT '0',
  `member3` int(11) DEFAULT '0',
  `member4` int(11) DEFAULT '0',
  `wins` int(11) DEFAULT '0',
  `losses` int(11) DEFAULT '0',
  `xp` int(11) DEFAULT '0',
  `level` int(11) DEFAULT '0',
  `rank` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `gneve1_BNET`
--

CREATE TABLE `gneve1_BNET` (
  `uid` int(11) NOT NULL DEFAULT '0',
  `acct_username` text,
  `username` text NOT NULL,
  `acct_userid` int(11) DEFAULT NULL,
  `acct_passhash1` text,
  `acct_email` text,
  `auth_admin` varchar(6) DEFAULT 'false',
  `auth_normallogin` varchar(6) DEFAULT 'true',
  `auth_changepass` varchar(6) DEFAULT 'true',
  `auth_changeprofile` varchar(6) DEFAULT 'true',
  `auth_botlogin` varchar(6) DEFAULT 'false',
  `auth_operator` varchar(6) DEFAULT 'false',
  `new_at_team_flag` int(11) DEFAULT '0',
  `auth_lock` varchar(1) DEFAULT '0',
  `auth_locktime` int(11) DEFAULT '0',
  `auth_lockreason` text,
  `auth_mute` varchar(1) DEFAULT '0',
  `auth_mutetime` int(11) DEFAULT '0',
  `auth_mutereason` text,
  `auth_command_groups` varchar(16) DEFAULT '1',
  `acct_lastlogin_time` int(11) DEFAULT '0',
  `acct_lastlogin_owner` varchar(16) DEFAULT NULL,
  `acct_lastlogin_clienttag` varchar(4) DEFAULT NULL,
  `acct_lastlogin_ip` varchar(16) DEFAULT NULL,
  `acct_statsdota_str3` text,
  `acct_statsdota_str2` text,
  `acct_statsdota_str1` text,
  `acct_ctime` varchar(128) DEFAULT NULL,
  `acct_hwid` text,
  `acct_verifier` text,
  `acct_salt` text,
  `acct_userlang` varchar(128) DEFAULT NULL,
  `acct_st_global_bantime` varchar(128) DEFAULT NULL,
  `acct_st_global_leavesstreak` varchar(128) DEFAULT NULL,
  `acct_st_dota88_assists` varchar(128) DEFAULT NULL,
  `acct_st_dota88_deaths` varchar(128) DEFAULT NULL,
  `acct_st_dota88_kills` varchar(128) DEFAULT NULL,
  `acct_st_dota88_maxstreak` varchar(128) DEFAULT NULL,
  `acct_st_dota88_minstreak` varchar(128) DEFAULT NULL,
  `acct_st_dota88_streak` varchar(128) DEFAULT NULL,
  `acct_st_dota88_leaves` varchar(128) DEFAULT NULL,
  `acct_st_dota88_loses` varchar(128) DEFAULT NULL,
  `acct_st_dota88_wins` varchar(128) DEFAULT NULL,
  `acct_st_dota88_mmr` varchar(128) DEFAULT NULL,
  `acct_st_dota88_FirstGame` varchar(128) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `gneve1_clan`
--

CREATE TABLE `gneve1_clan` (
  `cid` int(11) NOT NULL DEFAULT '0',
  `short` int(11) DEFAULT '0',
  `name` varchar(32) DEFAULT NULL,
  `motd` varchar(255) DEFAULT NULL,
  `creation_time` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `gneve1_clanmember`
--

CREATE TABLE `gneve1_clanmember` (
  `uid` int(11) NOT NULL DEFAULT '0',
  `cid` int(11) DEFAULT '0',
  `status` int(11) DEFAULT '0',
  `join_time` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `gneve1_friend`
--

CREATE TABLE `gneve1_friend` (
  `uid` int(11) NOT NULL DEFAULT '0',
  `count` varchar(128) DEFAULT NULL,
  `0_uid` varchar(128) DEFAULT NULL,
  `1_uid` varchar(128) DEFAULT NULL,
  `2_uid` varchar(128) DEFAULT NULL,
  `3_uid` varchar(128) DEFAULT NULL,
  `4_uid` varchar(128) DEFAULT NULL,
  `5_uid` varchar(128) DEFAULT NULL,
  `6_uid` varchar(128) DEFAULT NULL,
  `7_uid` varchar(128) DEFAULT NULL,
  `8_uid` varchar(128) DEFAULT NULL,
  `9_uid` varchar(128) DEFAULT NULL,
  `10_uid` varchar(128) DEFAULT NULL,
  `11_uid` varchar(128) DEFAULT NULL,
  `12_uid` varchar(128) DEFAULT NULL,
  `13_uid` varchar(128) DEFAULT NULL,
  `14_uid` varchar(128) DEFAULT NULL,
  `15_uid` varchar(128) DEFAULT NULL,
  `16_uid` varchar(128) DEFAULT NULL,
  `19_uid` varchar(128) DEFAULT NULL,
  `18_uid` varchar(128) DEFAULT NULL,
  `17_uid` varchar(128) DEFAULT NULL,
  `20_uid` varchar(128) DEFAULT NULL,
  `21_uid` varchar(128) DEFAULT NULL,
  `22_uid` varchar(128) DEFAULT NULL,
  `23_uid` varchar(128) DEFAULT NULL,
  `24_uid` varchar(128) DEFAULT NULL,
  `25_uid` varchar(128) DEFAULT NULL,
  `27_uid` varchar(128) DEFAULT NULL,
  `26_uid` varchar(128) DEFAULT NULL,
  `28_uid` varchar(128) DEFAULT NULL,
  `29_uid` varchar(128) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `gneve1_profile`
--

CREATE TABLE `gneve1_profile` (
  `uid` int(11) NOT NULL DEFAULT '0',
  `sex` varchar(8) DEFAULT NULL,
  `location` varchar(128) DEFAULT NULL,
  `description` varchar(256) DEFAULT NULL,
  `age` varchar(16) DEFAULT NULL,
  `clanname` varchar(48) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `gneve1_Record`
--

CREATE TABLE `gneve1_Record` (
  `uid` int(11) NOT NULL DEFAULT '0',
  `WAR3_solo_xp` int(11) DEFAULT '0',
  `WAR3_solo_level` int(11) DEFAULT '0',
  `WAR3_solo_wins` int(11) DEFAULT '0',
  `WAR3_solo_rank` int(11) DEFAULT '0',
  `WAR3_solo_losses` int(11) DEFAULT '0',
  `WAR3_team_xp` int(11) DEFAULT '0',
  `WAR3_team_level` int(11) DEFAULT '0',
  `WAR3_team_rank` int(11) DEFAULT '0',
  `WAR3_team_wins` int(11) DEFAULT '0',
  `WAR3_team_losses` int(11) DEFAULT '0',
  `WAR3_ffa_xp` int(11) DEFAULT '0',
  `WAR3_ffa_rank` int(11) DEFAULT '0',
  `WAR3_ffa_level` int(11) DEFAULT '0',
  `WAR3_ffa_wins` int(11) DEFAULT '0',
  `WAR3_ffa_losses` int(11) DEFAULT '0',
  `WAR3_orcs_wins` int(11) DEFAULT '0',
  `WAR3_orcs_losses` int(11) DEFAULT '0',
  `WAR3_humans_wins` int(11) DEFAULT '0',
  `WAR3_humans_losses` int(11) DEFAULT '0',
  `WAR3_undead_wins` int(11) DEFAULT '0',
  `WAR3_undead_losses` int(11) DEFAULT '0',
  `WAR3_nightelves_wins` int(11) DEFAULT '0',
  `WAR3_nightelves_losses` int(11) DEFAULT '0',
  `WAR3_random_wins` int(11) DEFAULT '0',
  `WAR3_random_losses` int(11) DEFAULT '0',
  `WAR3_teamcount` int(11) DEFAULT '0',
  `W3XP_solo_xp` int(11) DEFAULT '0',
  `W3XP_solo_level` int(11) DEFAULT '0',
  `W3XP_solo_wins` int(11) DEFAULT '0',
  `W3XP_solo_rank` int(11) DEFAULT '0',
  `W3XP_solo_losses` int(11) DEFAULT '0',
  `W3XP_team_xp` int(11) DEFAULT '0',
  `W3XP_team_level` int(11) DEFAULT '0',
  `W3XP_team_rank` int(11) DEFAULT '0',
  `W3XP_team_wins` int(11) DEFAULT '0',
  `W3XP_team_losses` int(11) DEFAULT '0',
  `W3XP_ffa_xp` int(11) DEFAULT '0',
  `W3XP_ffa_rank` int(11) DEFAULT '0',
  `W3XP_ffa_level` int(11) DEFAULT '0',
  `W3XP_ffa_wins` int(11) DEFAULT '0',
  `W3XP_ffa_losses` int(11) DEFAULT '0',
  `W3XP_orcs_wins` int(11) DEFAULT '0',
  `W3XP_orcs_losses` int(11) DEFAULT '0',
  `W3XP_humans_wins` int(11) DEFAULT '0',
  `W3XP_humans_losses` int(11) DEFAULT '0',
  `W3XP_undead_wins` int(11) DEFAULT '0',
  `W3XP_undead_losses` int(11) DEFAULT '0',
  `W3XP_nightelves_wins` int(11) DEFAULT '0',
  `W3XP_nightelves_losses` int(11) DEFAULT '0',
  `W3XP_random_wins` int(11) DEFAULT '0',
  `W3XP_random_losses` int(11) DEFAULT '0',
  `W3XP_teamcount` int(11) DEFAULT '0',
  `W3XP_userselected_icon` varchar(4) DEFAULT NULL,
  `W3XP_iconstash` varchar(256) DEFAULT NULL,
  `STAR_0_wins` int(11) DEFAULT '0',
  `STAR_0_losses` int(11) DEFAULT '0',
  `STAR_0_disconnects` int(11) DEFAULT '0',
  `STAR_1_wins` int(11) DEFAULT '0',
  `STAR_1_losses` int(11) DEFAULT '0',
  `STAR_1_disconnects` int(11) DEFAULT '0',
  `STAR_0_last_game` int(11) DEFAULT '0',
  `STAR_0_last_game_result` varchar(128) DEFAULT NULL,
  `STAR_1_last_game` int(11) DEFAULT '0',
  `STAR_1_last_game_result` varchar(128) DEFAULT NULL,
  `STAR_1_rating` int(11) DEFAULT '0',
  `STAR_1_high_rating` int(11) DEFAULT '0',
  `STAR_1_rank` int(11) DEFAULT '0',
  `STAR_1_high_rank` int(11) DEFAULT '0',
  `SEXP_0_wins` int(11) DEFAULT '0',
  `SEXP_0_losses` int(11) DEFAULT '0',
  `SEXP_0_disconnects` int(11) DEFAULT '0',
  `SEXP_1_wins` int(11) DEFAULT '0',
  `SEXP_1_losses` int(11) DEFAULT '0',
  `SEXP_1_disconnects` int(11) DEFAULT '0',
  `SEXP_0_last_game` int(11) DEFAULT '0',
  `SEXP_0_last_game_result` varchar(128) DEFAULT NULL,
  `SEXP_1_last_game` int(11) DEFAULT '0',
  `SEXP_1_last_game_result` varchar(128) DEFAULT NULL,
  `SEXP_1_rating` int(11) DEFAULT '0',
  `SEXP_1_high_rating` int(11) DEFAULT '0',
  `SEXP_1_rank` int(11) DEFAULT '0',
  `SEXP_1_high_rank` int(11) DEFAULT '0',
  `SEXP_userselected_icon` varchar(4) DEFAULT NULL,
  `SEXP_iconstash` varchar(256) DEFAULT NULL,
  `TSUN_solo_rank` int(11) DEFAULT '0',
  `TSUN_solo_points` int(11) DEFAULT '0',
  `TSUN_solo_wins` int(11) DEFAULT '0',
  `TSUN_solo_losses` int(11) DEFAULT '0',
  `TSUN_solo_disconnects` int(11) DEFAULT '0',
  `TSXP_solo_rank` int(11) DEFAULT '0',
  `TSXP_solo_points` int(11) DEFAULT '0',
  `TSXP_solo_wins` int(11) DEFAULT '0',
  `TSXP_solo_losses` int(11) DEFAULT '0',
  `TSXP_solo_disconnects` int(11) DEFAULT '0',
  `RAL2_solo_rank` int(11) DEFAULT '0',
  `RAL2_solo_points` int(11) DEFAULT '0',
  `RAL2_solo_wins` int(11) DEFAULT '0',
  `RAL2_solo_losses` int(11) DEFAULT '0',
  `RAL2_solo_disconnects` int(11) DEFAULT '0',
  `YURI_solo_rank` int(11) DEFAULT '0',
  `YURI_solo_points` int(11) DEFAULT '0',
  `YURI_solo_wins` int(11) DEFAULT '0',
  `YURI_solo_losses` int(11) DEFAULT '0',
  `YURI_solo_disconnects` int(11) DEFAULT '0',
  `W3XP_w3pgrace` varchar(128) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `gneve1_WOL`
--

CREATE TABLE `gneve1_WOL` (
  `uid` int(11) NOT NULL DEFAULT '0',
  `auth_apgar` varchar(8) DEFAULT NULL,
  `acct_locale` int(11) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Индексы сохранённых таблиц
--

--
-- Индексы таблицы `gneve1_arrangedteam`
--
ALTER TABLE `gneve1_arrangedteam`
  ADD PRIMARY KEY (`teamid`);

--
-- Индексы таблицы `gneve1_BNET`
--
ALTER TABLE `gneve1_BNET`
  ADD PRIMARY KEY (`uid`);

--
-- Индексы таблицы `gneve1_clan`
--
ALTER TABLE `gneve1_clan`
  ADD PRIMARY KEY (`cid`);

--
-- Индексы таблицы `gneve1_clanmember`
--
ALTER TABLE `gneve1_clanmember`
  ADD PRIMARY KEY (`uid`),
  ADD KEY `cid` (`cid`);

--
-- Индексы таблицы `gneve1_friend`
--
ALTER TABLE `gneve1_friend`
  ADD PRIMARY KEY (`uid`);

--
-- Индексы таблицы `gneve1_profile`
--
ALTER TABLE `gneve1_profile`
  ADD PRIMARY KEY (`uid`);

--
-- Индексы таблицы `gneve1_Record`
--
ALTER TABLE `gneve1_Record`
  ADD PRIMARY KEY (`uid`);

--
-- Индексы таблицы `gneve1_WOL`
--
ALTER TABLE `gneve1_WOL`
  ADD PRIMARY KEY (`uid`);
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
