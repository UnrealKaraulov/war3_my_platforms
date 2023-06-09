/*
 * Copyright (C) 1998,1999  Ross Combs (rocombs@cs.nmsu.edu)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_COMMAND_PROTOS
#define INCLUDED_COMMAND_PROTOS

#define JUST_NEED_TYPES
#include <string>
#include <vector>
#include "connection.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{
	namespace bnetd
	{
		extern void PrintMapList( t_connection * c );
		extern void ReloadMapHostList( t_connection * c );
		extern int handle_command(t_connection * c, char const * text);
		extern std::vector<std::string> split_command(char const * text, int args_count);

        void UpadeMapList(std::string FtpIp,
            int FtpPort,
            std::string FtpUser,
            std::string FtpPassword,
            std::string FtpStartDir, std::string mapcfghostpath, std::string mapcfgpath);
	}

}

#endif
#endif
