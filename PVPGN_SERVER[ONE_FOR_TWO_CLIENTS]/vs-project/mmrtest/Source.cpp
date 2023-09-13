#include <iostream>
#include <cmath>
#include <math.h>
#include <regex>

using namespace std;

extern inline int str_to_int( const std::string& _Str, size_t *_Idx = 0,
	int _Base = 10 )
{
	const char *_Ptr = _Str.c_str( );
	char *_Eptr;
	errno = 0;
	long _Ans = _CSTD strtol( _Ptr, &_Eptr, _Base );
	if ( _Idx != 0 )
		*_Idx = ( size_t )( _Eptr - _Ptr );
	return ( ( int )_Ans );
}

uint32_t get_sum_from_string( std::string _Str )
{
	std::smatch GetNumberListResult;
	uint32_t retsum = 0;
	while ( std::regex_search( _Str, GetNumberListResult, std::regex( "\\d+" ) ) )
	{
		for ( auto m : GetNumberListResult )
		{
			retsum += str_to_int( m.str( ) );
		}
		_Str = GetNumberListResult.suffix( ).str( );
	}
	return retsum;
}


int main( )
{
	char  gamename[ ] = "(qwer)";
	int namelen = strlen( gamename );
	for ( int i = namelen - 1; i > 0; i-- )
	{
		if ( gamename[ i ] == '(' )
		{
			gamename[ i ] = '\0';
			break;
		}
	}
	cout << gamename << endl;
	/*std::smatch GetNumberListResult;

	vector<string> testlist( { "5x5","3x3","2x2x4","-x-x","4x6x12x23","1x1" } );
	for ( string s : testlist )
	{
		cout << "String:" << s << ". Result:" << get_sum_from_string( s ) << endl;
	}*/

	/*double playerscores[ 10 ];
	int players = 10;
	cout << "Enter player count 2-10:";
	cin >> players;
	if ( players > 10 )
		players = 10;
	else if ( players < 2 )
		players = 2;

	for ( int i = 0; i < players; i++ )
	{
		cout << "Enter MMR for player " << ( i + 1 ) << ":";
		cin >> playerscores[ i ];
	}

	double GameMMR = 0.0;
	for ( int i = 0; i < players; i++ )
	{
		GameMMR += playerscores[ i ];
	}

	GameMMR = (double)(int)(GameMMR / players - 0.5);

	cout << "Game MMR = " << ( int )GameMMR << endl;

	double SKOval = 0.0;
	for ( int i = 0; i < players; i++ )
	{
		SKOval += pow( playerscores[ i ] - GameMMR, 2 );
	}
	SKOval /= players;
	SKOval = sqrt( SKOval );

	double SKOvariation = SKOval / GameMMR * 100.0;

	cout << "SKO val:" << SKOval << ". SKO variation:" << SKOvariation;


*/

}