//////////////////////////////////////////////////////////////////////////
/// Mike Brown, 2022
///
///	Filename: 	OutputStamp.cpp
///	Created:	30/06/2022
///	Author:		Mike Brown
///	
///	Description:
///
//////////////////////////////////////////////////////////////////////////

#include <iomanip>
#include <sstream>
#if defined(__linux)
#include<cstring>
#endif // #if defined(__linux)
#include "OutputStamp.h"
#include "Utilities/Strings.h"

namespace mbp
{
	namespace streams
	{
		inline uint64_t GetNowMillisec()
		{
			auto timeNow = std::chrono::steady_clock::now();
			uint64_t nowMillisec = std::chrono::duration_cast< std::chrono::milliseconds >( timeNow.time_since_epoch() ).count();
			uint64_t flooredToSec = std::chrono::duration_cast< std::chrono::seconds >( timeNow.time_since_epoch() ).count();
			uint64_t flooredInMilli = flooredToSec * 1000;
			nowMillisec = nowMillisec - flooredInMilli;
			return nowMillisec;
		}

		template<>
		int SystemTimeStamp_t< char >::WriteStamp( void * ptr_ /*= nullptr */ )
		{
			std::stringstream theTimeNow;

			const std::chrono::time_point<std::chrono::system_clock> now =
				std::chrono::system_clock::now();
			const std::time_t t_c = std::chrono::system_clock::to_time_t( now );

#if defined( _MSC_VER )
			tm myTime;
			localtime_s( &myTime, &t_c );
			theTimeNow << std::put_time( &myTime, "%F %T:" ) << std::setw( 3 ) << std::setfill( '0' ) << GetNowMillisec() << " ";
#else
			theTimeNow << std::put_time( std::localtime( &t_c ), "%F %T:" ) << std::setw( 3 ) << std::setfill( '0' ) << GetNowMillisec() << " ";
#endif
			size_t numCharacters = theTimeNow.str().length();
			if ( ptr_ )
				memcpy( ptr_, theTimeNow.str().c_str(), numCharacters );
			return static_cast< int >( numCharacters );
		}

		template<>
		int SystemTimeStamp_t< char32_t >::WriteStamp( void * ptr_ /*= nullptr */ )
		{
			// this function is placeholder and very SLOW because numerics don't work natively with this character type
			char32_t buffer[ 1024 ]; // for UTF32 conversion

			std::stringstream theTimeNow;

			const std::chrono::time_point<std::chrono::system_clock> now =
				std::chrono::system_clock::now();
			const std::time_t t_c = std::chrono::system_clock::to_time_t( now );

#if defined( _MSC_VER )
			tm myTime;
			localtime_s( &myTime, &t_c );
			theTimeNow << std::put_time( &myTime, "%F %T:" ) << std::setw( 3 ) << std::setfill( '0' ) << GetNowMillisec() << " ";
#else
			theTimeNow << std::put_time( std::localtime( &t_c ), "%F %T:" ) << std::setw( 3 ) << std::setfill( '0' ) << GetNowMillisec() << " ";
#endif
			size_t numCharacters = theTimeNow.str().length();
			if ( ptr_ )
			{
				// time is most likely OK to just widen, but some locales may have unicode codepoints so we'll do it the right way
				strings::UTF8ToUTF32( reinterpret_cast< const char * >( theTimeNow.str().c_str() ), buffer );
				memcpy( ptr_, buffer, numCharacters * sizeof( char32_t ) );
			}
			return static_cast< int >( numCharacters );

		}

		template<>
		int SystemTimeStamp_t< char16_t >::WriteStamp( void * ptr_ /*= nullptr */ )
		{
			// this function is placeholder and very SLOW because numerics don't work natively with this character type
			char16_t buffer[ 1024 ]; // for conversion

			std::stringstream theTimeNow;

			const std::chrono::time_point<std::chrono::system_clock> now =
				std::chrono::system_clock::now();
			const std::time_t t_c = std::chrono::system_clock::to_time_t( now );

#if defined( _MSC_VER )
			tm myTime;
			localtime_s( &myTime, &t_c );
			theTimeNow << std::put_time( &myTime, "%F %T:" ) << std::setw( 3 ) << std::setfill( '0' ) << GetNowMillisec() << " ";
#else
			theTimeNow << std::put_time( std::localtime( &t_c ), "%F %T:" ) << std::setw( 3 ) << std::setfill( '0' ) << GetNowMillisec() << " ";
#endif
			size_t numCharacters = theTimeNow.str().length();
			if ( ptr_ )
			{
				// time is most likely OK to just widen, but some locales may have unicode codepoints so we'll do it the right way
				strings::UTF8ToUTF16( reinterpret_cast< const char * >( theTimeNow.str().c_str() ), buffer );
				memcpy( ptr_, buffer, numCharacters * sizeof( char16_t ) );
			}
			return static_cast< int >( numCharacters );

		}

		template<>
		int SystemTimeStamp_t< wchar_t >::WriteStamp( void * ptr_ /*= nullptr */ )
		{
			std::wstringstream theTimeNow;

			const std::chrono::time_point<std::chrono::system_clock> now =
				std::chrono::system_clock::now();
			const std::time_t t_c = std::chrono::system_clock::to_time_t( now );

#if defined( _MSC_VER )
			tm myTime;
			localtime_s( &myTime, &t_c );
			theTimeNow << std::put_time( &myTime, L"%F %T:" ) << std::setw( 3 ) << std::setfill( L'0' ) << GetNowMillisec() << L" ";
#else
			theTimeNow << std::put_time( std::localtime( &t_c ), L"%F %T:" ) << std::setw( 3 ) << std::setfill( L'0' ) << GetNowMillisec() << L" ";
#endif
			size_t numCharacters = ( theTimeNow.str().length() );
			if ( ptr_ )
				memcpy( ptr_, theTimeNow.str().c_str(), numCharacters * sizeof( wchar_t ) );
			return static_cast< int >( numCharacters );
		}
	}
}