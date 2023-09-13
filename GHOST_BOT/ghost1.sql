-- phpMyAdmin SQL Dump
-- version 4.7.3
-- https://www.phpmyadmin.net/
--
-- Хост: 127.0.0.1:3306
-- Время создания: Ноя 05 2017 г., 17:59
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
-- База данных: `ghost1`
--

-- --------------------------------------------------------

--
-- Структура таблицы `admins`
--

CREATE TABLE `admins` (
  `id` int(11) NOT NULL,
  `botid` int(11) NOT NULL,
  `name` varchar(15) NOT NULL,
  `server` varchar(100) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `bans`
--

CREATE TABLE `bans` (
  `id` int(11) NOT NULL,
  `botid` int(11) NOT NULL,
  `server` varchar(100) NOT NULL,
  `name` varchar(15) NOT NULL,
  `ip` varchar(15) NOT NULL,
  `date` datetime NOT NULL,
  `gamename` varchar(31) NOT NULL,
  `admin` varchar(15) NOT NULL,
  `reason` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `dotagames`
--

CREATE TABLE `dotagames` (
  `id` int(11) NOT NULL,
  `botid` int(11) NOT NULL,
  `gameid` int(11) NOT NULL,
  `winner` int(11) NOT NULL,
  `min` int(11) NOT NULL,
  `sec` int(11) NOT NULL,
  `firstblood` int(11) NOT NULL,
  `statstype` varchar(100) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `dotaplayers`
--

CREATE TABLE `dotaplayers` (
  `id` int(11) NOT NULL,
  `botid` int(11) NOT NULL,
  `gameid` int(11) NOT NULL,
  `colour` int(11) NOT NULL,
  `kills` int(11) NOT NULL,
  `deaths` int(11) NOT NULL,
  `creepkills` int(11) NOT NULL,
  `creepdenies` int(11) NOT NULL,
  `assists` int(11) NOT NULL,
  `gold` int(11) NOT NULL,
  `neutralkills` int(11) NOT NULL,
  `item1` char(4) NOT NULL,
  `item2` char(4) NOT NULL,
  `item3` char(4) NOT NULL,
  `item4` char(4) NOT NULL,
  `item5` char(4) NOT NULL,
  `item6` char(4) NOT NULL,
  `hero` char(4) NOT NULL,
  `herolevel` int(11) NOT NULL,
  `newcolour` int(11) NOT NULL,
  `towerkills` int(11) NOT NULL,
  `raxkills` int(11) NOT NULL,
  `courierkills` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `dota_elo_games_scored`
--

CREATE TABLE `dota_elo_games_scored` (
  `id` int(11) NOT NULL,
  `gameid` int(11) NOT NULL,
  `duration` int(11) NOT NULL,
  `gamerank` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `dota_elo_scores`
--

CREATE TABLE `dota_elo_scores` (
  `id` int(11) NOT NULL,
  `name` varchar(15) NOT NULL,
  `server` varchar(100) NOT NULL,
  `score` double NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `downloads`
--

CREATE TABLE `downloads` (
  `id` int(11) NOT NULL,
  `botid` int(11) NOT NULL,
  `map` varchar(100) NOT NULL,
  `mapsize` int(11) NOT NULL,
  `datetime` datetime NOT NULL,
  `name` varchar(15) NOT NULL,
  `ip` varchar(15) NOT NULL,
  `spoofed` int(11) NOT NULL,
  `spoofedrealm` varchar(100) NOT NULL,
  `downloadtime` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `gameplayers`
--

CREATE TABLE `gameplayers` (
  `id` int(11) NOT NULL,
  `botid` int(11) NOT NULL,
  `gameid` int(11) NOT NULL,
  `name` varchar(15) NOT NULL,
  `ip` varchar(15) NOT NULL,
  `spoofed` int(11) NOT NULL,
  `reserved` int(11) NOT NULL,
  `loadingtime` int(11) NOT NULL,
  `left` int(11) NOT NULL,
  `leftreason` varchar(100) NOT NULL,
  `team` int(11) NOT NULL,
  `colour` int(11) NOT NULL,
  `spoofedrealm` varchar(100) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `games`
--

CREATE TABLE `games` (
  `id` int(11) NOT NULL,
  `botid` int(11) NOT NULL,
  `server` varchar(100) NOT NULL,
  `map` varchar(100) NOT NULL,
  `datetime` datetime NOT NULL,
  `gamename` varchar(31) NOT NULL,
  `ownername` varchar(15) NOT NULL,
  `duration` int(11) NOT NULL,
  `gamestate` int(11) NOT NULL,
  `creatorname` varchar(15) NOT NULL,
  `creatorserver` varchar(100) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `scores`
--

CREATE TABLE `scores` (
  `id` int(11) NOT NULL,
  `category` varchar(25) NOT NULL,
  `name` varchar(15) NOT NULL,
  `server` varchar(100) NOT NULL,
  `score` double NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `w3mmdplayers`
--

CREATE TABLE `w3mmdplayers` (
  `id` int(11) NOT NULL,
  `botid` int(11) NOT NULL,
  `category` varchar(25) NOT NULL,
  `gameid` int(11) NOT NULL,
  `pid` int(11) NOT NULL,
  `name` varchar(15) NOT NULL,
  `flag` varchar(32) NOT NULL,
  `leaver` int(11) NOT NULL,
  `practicing` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `w3mmdvars`
--

CREATE TABLE `w3mmdvars` (
  `id` int(11) NOT NULL,
  `botid` int(11) NOT NULL,
  `gameid` int(11) NOT NULL,
  `pid` int(11) NOT NULL,
  `varname` varchar(25) NOT NULL,
  `value_int` int(11) DEFAULT NULL,
  `value_real` double DEFAULT NULL,
  `value_string` varchar(100) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Индексы сохранённых таблиц
--

--
-- Индексы таблицы `admins`
--
ALTER TABLE `admins`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `bans`
--
ALTER TABLE `bans`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `dotagames`
--
ALTER TABLE `dotagames`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `dotaplayers`
--
ALTER TABLE `dotaplayers`
  ADD PRIMARY KEY (`id`),
  ADD KEY `gameid` (`gameid`,`colour`);

--
-- Индексы таблицы `dota_elo_games_scored`
--
ALTER TABLE `dota_elo_games_scored`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `dota_elo_scores`
--
ALTER TABLE `dota_elo_scores`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `downloads`
--
ALTER TABLE `downloads`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `gameplayers`
--
ALTER TABLE `gameplayers`
  ADD PRIMARY KEY (`id`),
  ADD KEY `gameid` (`gameid`);

--
-- Индексы таблицы `games`
--
ALTER TABLE `games`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `scores`
--
ALTER TABLE `scores`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `w3mmdplayers`
--
ALTER TABLE `w3mmdplayers`
  ADD PRIMARY KEY (`id`);

--
-- Индексы таблицы `w3mmdvars`
--
ALTER TABLE `w3mmdvars`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT для сохранённых таблиц
--

--
-- AUTO_INCREMENT для таблицы `admins`
--
ALTER TABLE `admins`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `bans`
--
ALTER TABLE `bans`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `dotagames`
--
ALTER TABLE `dotagames`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `dotaplayers`
--
ALTER TABLE `dotaplayers`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `dota_elo_games_scored`
--
ALTER TABLE `dota_elo_games_scored`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `dota_elo_scores`
--
ALTER TABLE `dota_elo_scores`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `downloads`
--
ALTER TABLE `downloads`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `gameplayers`
--
ALTER TABLE `gameplayers`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `games`
--
ALTER TABLE `games`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `scores`
--
ALTER TABLE `scores`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `w3mmdplayers`
--
ALTER TABLE `w3mmdplayers`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT для таблицы `w3mmdvars`
--
ALTER TABLE `w3mmdvars`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
