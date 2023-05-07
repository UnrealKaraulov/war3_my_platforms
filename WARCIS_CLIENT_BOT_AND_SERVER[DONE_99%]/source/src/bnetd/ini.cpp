/*
* This is an independent project of an individual developer. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
#include "ini.h"

std::string trim( const std::string& str,
	const std::string& whitespace = " \t\"" )
{
	const auto strBegin = str.find_first_not_of( whitespace );
	if ( strBegin == std::string::npos )
		return ""; // no content

	const auto strEnd = str.find_last_not_of( whitespace );
	const auto strRange = strEnd - strBegin + 1;

	return str.substr( strBegin, strRange );
}

namespace nc {

	 std::string emptystring = "";
	 IniNode emptynode = IniNode( );
	Ini::Ini( const std::string & _File ) : file( _File )
	{
	}

	Ini::Ini( const Ini & _Other )
	{
		file = _Other.file;
	}

	Ini::~Ini( )
	{
	}

	IniNode& Ini::operator[]( const std::string & _NodeName )
	{
		printf( "Get node:%s", _NodeName.c_str( ) );
		if ( nodes.size( ) > 0 && nodes.find( _NodeName ) != nodes.end( ) )
			return nodes[ _NodeName ];
		else
			return emptynode;
	}

	void Ini::Save( )
	{
		if ( nodes.size( ) < 1 )
			return;

		if ( fs.is_open( ) )
			fs.close( );

		fs.open( file, std::ios::out | std::ios::trunc );

		for ( auto it = nodes.begin( ); it != nodes.end( ); it++ ) {
			fs << "[" << it->first << "]\n";

			if ( it->second.values.size( ) < 1 )
				continue;

			for ( auto it2 = it->second.values.begin( ); it2 != it->second.values.end( );
				it2++ ) {
				fs << it2->first;

				if ( !it2->second.empty( ) )
					fs << "=" << it2->second;
				fs << "\n";
			}

			fs << "\n";
		}
	}

	bool Ini::Load( )
	{
		if ( fs.is_open( ) ) {
			fs.close( );
		}

		fs.open( file,  std::ios::in );

		if ( !fs.is_open( ) ) {
			return false;
		}

		std::string line, token;

		while ( std::getline( fs, line ) ) {
			if ( line.length( ) < 3 )
				continue;

			if ( line.front( ) == '[' && line.find( ']' ) != std::string::npos ) {
				token = line.substr( 1, line.find( ']' ) - 1 );
			

				printf( "Found new section:%s\n", token.c_str( ) );
				IniNode node;
				node.name = trim(token);
				node.owner = this;

				while ( std::getline( fs, line ) ) {
					if ( line.length( ) < 3 )
						continue;

					if ( line.front( ) == '[' && line.find( ']' ) != std::string::npos ) {
						nodes[ token ] = node;
						token = node.name = trim( line.substr( 1, line.find( ']' ) - 1 ) );
						printf( "Found new section #2:%s\n", token.c_str( ) );
					}
					else if ( line.front( ) != '#' && line.front( ) != ';' ) {
						auto pair = Split( line, '=' );

						printf( "Found new value:[%s] %s = %s\n", token.c_str( ),pair.first.c_str( ), pair.second.c_str( ) );

						node.Set( trim( pair.first), trim( pair.second) );
					}
				}

				nodes[ token ] = node;
			}
		}

		return true;
	}

	const std::string & IniNode::operator[]( const std::string & _ValueName )
	{
		return Get( _ValueName );
	}

	void IniNode::Set( const std::string & _Name, const std::string & _Value )
	{
		values[ _Name ] = _Value;
	}

	const std::string & IniNode::Get( const std::string & _Name )
	{
		if ( values.size( ) > 0 && values.find( _Name ) != values.end( ) )
			return values[ _Name ];
		else
			return emptystring;
	}
} // namespace nc