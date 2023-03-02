//////////////////////////////////////////////////////////////////////////
/// Mike Brown, 2014-2022
///
///	Filename: 	OutputStreams.cpp
///	Created:	31/12/2014
///	Author:		Mike Brown
///	
///	Description:
///
//////////////////////////////////////////////////////////////////////////

#include "OutputStreams.h"

namespace mbp
{
	namespace streams
	{

		//////////////////////////////////////////////////////////////////////////
		// Default ChannelSettings for Streams and Channels
		//////////////////////////////////////////////////////////////////////////

		StreamSettings & GetDefaultChannelSettings()
		{
			static StreamSettings kDefaultSettings
			{
				1,					// Stream/Channel enable flag
				kPriorityDefault,	// default priority level
				kDefaultFilter		// default output filter level (priority > cap = fail filter test)
			};
			return kDefaultSettings;
		}

		//////////////////////////////////////////////////////////////////////////
		// Conversion functions called by ConvertingStream_t operator <<
		//////////////////////////////////////////////////////////////////////////

		// assume UTF32 text and transform into UTF8 
		ConvertingStream_t< char > & ConvertText( ConvertingStream_t< char > & stream_, char32_t const * pSource_ )
		{
			char buf[ 4 ];
			char32_t cp;
			size_t count;
			for( ; cp = *pSource_++; )
			{
				count = strings::NumUTF8CharsFromCodepoint( cp );
				strings::GetUTF8FromCodePoint( cp, buf );
				stream_.rdbuf()->sputn( buf, count );
			}
			return stream_;
		}

		// assume UTF32 and transform into UTF16
		ConvertingStream_t< char16_t > & ConvertText( ConvertingStream_t< char16_t> & stream_, char32_t const * pSource_ )
		{
			char16_t buf[ 2 ];
			char32_t cp;
			size_t count;
			for ( ; cp = *pSource_++; )
			{
				count = strings::NumUTF16CharsFromCodepoint( cp );
				strings::GetUTF16FromCodePoint( cp, buf );
				stream_.rdbuf()->sputn( buf, count );
			}
			return stream_;
		}

		// assume UTF16 and transform into UTF8
		ConvertingStream_t< char > & ConvertText( ConvertingStream_t< char > & stream_, char16_t const * pSource_ )
		{
			char32_t cp;
			char buf[ 4 ];
			size_t count;	
			while ( *pSource_ )
			{
				count = strings::GetCodepointAndBytesFromUTF16( pSource_, cp );
				pSource_ += count >> 1;
				count = strings::NumUTF8CharsFromCodepoint( cp );
				strings::GetUTF8FromCodePoint( cp, buf );
				stream_.rdbuf()->sputn( buf, count );
			}
			return stream_;
		}

		// assume UTF8 and transform to UTF32
		ConvertingStream_t< char32_t > & ConvertText( ConvertingStream_t< char32_t > & stream_, char const * pSource_ )
		{
			char32_t cp;
			size_t count;
			while ( *pSource_ )
			{
				count = strings::GetCodepointAndCountFromUTF8( pSource_, cp );
				pSource_ += count;
				stream_.rdbuf()->sputc( cp );
			}
			return stream_;
		}

		// assume UTF8 and transform to UTF16
		ConvertingStream_t< char16_t> & ConvertText( ConvertingStream_t< char16_t > & stream_, char const * pSource_ )
		{
			char16_t buff[ 3 ];
			char32_t cp;
			size_t count, numChars;
			while ( *pSource_ )
			{
				count = strings::GetCodepointAndCountFromUTF8( pSource_, cp );
				pSource_ += count;
				strings::GetUTF16FromCodePoint( cp, buff );
				numChars = strings::NumUTF16CharsFromCodepoint( cp );
				stream_.rdbuf()->sputn( buff, numChars );
			}
			return stream_;
		}

		// assume UTF16 and transform to UTF32
		ConvertingStream_t< char32_t > & ConvertText( ConvertingStream_t< char32_t > & stream_, char16_t const * pSource_ )
		{
			char32_t cp;
			size_t count;
			while( *pSource_ )
			{
				count = strings::GetCodepointAndBytesFromUTF16( pSource_, cp );
				pSource_ += count >> 1;
				stream_.rdbuf()->sputc( cp );
			}
			return stream_;
		}

		//////////////////////////////////////////////////////////////////////////
		// Platform dependent versions
		//////////////////////////////////////////////////////////////////////////

#if defined( __linux )
		// assume UTF8 source and transform to UTF32 wchar_t
		ConvertingStream_t< wchar_t > & ConvertText( ConvertingStream_t< wchar_t > & stream_, char const * pSource_ )
		{
			char32_t cp;
			size_t count;
			while ( *pSource_ )
			{
				count = strings::GetCodepointAndCountFromUTF8( pSource_, cp );
				pSource_ += count;
				stream_.rdbuf()->sputc( cp );
			}
			return stream_;
		}
		// assume UTF16 source and transform to UTF32 wchar_t
		ConvertingStream_t< wchar_t > & ConvertText( ConvertingStream_t< wchar_t > & stream_, char16_t const * pSource_ )
		{
			char32_t cp;
			char16_t code;
			size_t count;
			while ( *pSource_ )
			{
				count = strings::GetCodepointAndBytesFromUTF16( pSource_, cp );
				pSource_ += count >> 1;
				stream_.rdbuf()->sputc( cp );
			}
			return stream_;
		}
		// assume UTF32 source and transform to UTF16
		ConvertingStream_t< char16_t > & ConvertText( ConvertingStream_t< char16_t > & stream_, wchar_t const * pSource_ )
		{
			char16_t buf[ 2 ];
			char32_t cp;
			size_t count;
			for ( ; cp = *pSource_++; )
			{
				count = strings::NumUTF16CharsFromCodepoint( cp );
				strings::GetUTF16FromCodePoint( cp, buf );
				stream_.rdbuf()->sputn( buf, count );
			}
			return stream_;
		}
		// assume UTF32 source and transform to UTF8
		ConvertingStream_t< char > & ConvertText( ConvertingStream_t< char > & stream_, wchar_t const * pSource_ )
		{
			char buf[ 4 ];
			char32_t cp;
			size_t count;
			for ( ; cp = *pSource_++; )
			{
				count = strings::NumUTF8CharsFromCodepoint( cp );
				strings::GetUTF8FromCodePoint( cp, buf );
				stream_.rdbuf()->sputn( buf, count );
			}
			return stream_;
		}
#elif defined ( _MSC_VER )
		// assume UTF8 and transform to UTF16 wchar_t
		ConvertingStream_t< wchar_t > & ConvertText( ConvertingStream_t< wchar_t > & stream_, char const * pSource_ )
		{
			char16_t buff[ 3 ];
			char32_t cp;
			size_t count, numChars;
			while ( *pSource_ )
			{
				count = strings::GetCodepointAndCountFromUTF8( pSource_, cp );
				pSource_ += count;
				strings::GetUTF16FromCodePoint( cp, buff );
				numChars = strings::NumUTF16CharsFromCodepoint( cp );
				stream_.rdbuf()->sputn( reinterpret_cast< wchar_t const * >( buff ), numChars );
			}
			return stream_;
		}
		// assume UTF32 and transform to UTF16 wchar_t
		ConvertingStream_t< wchar_t > & ConvertText( ConvertingStream_t< wchar_t > & stream_, char32_t const * pSource_ )
		{
			char16_t buf[ 2 ];
			char32_t cp;
			size_t count;
			for ( ; cp = *pSource_++; )
			{
				count = strings::NumUTF16CharsFromCodepoint( cp );
				strings::GetUTF16FromCodePoint( cp, buf );
				stream_.rdbuf()->sputn( reinterpret_cast< wchar_t const * >( buf ), count );
			}
			return stream_;
		}	
		// assume UTF16 and transform to UTF8
		ConvertingStream_t< char > & ConvertText( ConvertingStream_t< char > & stream_, wchar_t const * pSource_ )
		{
			char32_t cp;
			char buf[ 4 ];
			size_t count;
			while ( *pSource_ )
			{
				count = strings::GetCodepointAndBytesFromUTF16( reinterpret_cast< char16_t const * >( pSource_ ), cp );
				pSource_ += count >> 1;
				count = strings::NumUTF8CharsFromCodepoint( cp );
				strings::GetUTF8FromCodePoint( cp, buf );
				stream_.rdbuf()->sputn( buf, count );
			}
			return stream_;
		}
		// assume UTF16 and transform to UTF32
		ConvertingStream_t< char32_t > & ConvertText( ConvertingStream_t< char32_t > &stream_, wchar_t const * pSource_ )
		{
			char32_t cp;
			size_t count;
			while ( *pSource_ )
			{
				count = strings::GetCodepointAndBytesFromUTF16( reinterpret_cast< char16_t const * >( pSource_ ), cp );
				pSource_ += count >> 1;
				stream_.rdbuf()->sputc( cp );
			}
			return stream_;
		}
#endif //#if defined( __linux )...
	}
}
