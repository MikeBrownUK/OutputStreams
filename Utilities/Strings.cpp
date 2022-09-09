//////////////////////////////////////////////////////////////////////////
/// Mike Brown, 2022
///
/// Filename:	Strings.cpp
/// Created:	19/8/2022
/// Author:		Mike
/// 
/// Description: Various string manipulators
/// 			 Endianness of platform and source are assumed identical
///
//////////////////////////////////////////////////////////////////////////

#include "Strings.h"

namespace mbp
{
	namespace strings
	{
		size_t GetUTF8StringLengthInCharacters( void const * pSource_ )
		{
			size_t count = 0;
			char const * ptr = reinterpret_cast< char const * >( pSource_ );
			while ( *ptr++ )
			{
				++count;
				// 2 byte encoding
				if ( ( *( ptr - 1 ) & 0xE0 ) == 0xC0 && ( *( ptr ) ) && ( *( ptr ) & 0xC0 ) == 0x80 )
				{
					++ptr;
				}
				// 3 byte
				else if ( ( *( ptr - 1 ) & 0xF0 ) == 0xE0 && ( *ptr ) && ( *( ptr ) & 0xC0 ) == 0x80 && ( *ptr + 1 ) && ( *( ptr + 1 ) & 0xC0 ) == 0x80 )
				{
					ptr += 2;
				}
				// 4 byte
				else if ( ( *( ptr - 1 ) & 0xF8 ) == 0xF0 && ( *ptr ) && ( *ptr & 0xC0 ) == 0x80 && ( *ptr + 1 ) && ( *( ptr + 1 ) & 0xC0 ) == 0x80 && ( *ptr + 2 ) && ( *( ptr + 2 ) & 0xC0 ) == 0x80 )
				{
					ptr += 3;
				}
			}
			return count;
		}

		size_t GetUTF8StringLengthInBytes( void const * pSource_ )
		{
			size_t count = 0;
			char const * ptr = reinterpret_cast< char const * >( pSource_ );
			while ( *ptr++ )
			{
				++count;
				// 2 byte encoding
				if ( ( *( ptr - 1 ) & 0xE0 ) == 0xC0 && ( *( ptr ) ) && ( *ptr & 0xC0 ) == 0x80 )
				{
					++count;
					++ptr;
				}
				// 3 byte
				else if ( ( *( ptr - 1 ) & 0xF0 ) == 0xE0 && ( *ptr ) && ( *ptr & 0xC0 ) == 0x80 && ( *ptr + 1 ) && ( *( ptr + 1 ) & 0xC0 ) == 0x80 )
				{
					count += 2;
					ptr += 2;
				}
				// 4 byte
				else if ( ( *( ptr - 1 ) & 0xF8 ) == 0xF0 && ( *ptr ) && ( *ptr & 0xC0 ) == 0x80 && ( *( ptr + 1 ) ) && ( *( ptr + 1 ) & 0xC0 ) == 0x80 && ( *( ptr + 2 ) ) && ( *( ptr + 2 ) & 0xC0 ) == 0x80 )
				{
					count += 3;
					ptr += 3;
				}
			}
			return count;
		}

		size_t GetUTF16StringLengthInCharacters( void const * pSource_ )
		{
			// use char16_t because wchar_t on linux is 4 bytes
			size_t count = 0;
			char16_t const * ptr = reinterpret_cast< char16_t const * >( pSource_ );
			while ( *ptr++ )
			{
				++count;
				if ( ( *( ptr - 1 ) >= 0xD800 ) && ( *( ptr - 1 ) < 0xDC00 ) && *ptr && ( *ptr >= 0xDC00 ) && ( *ptr < 0xE000 ) )
					++ptr;
			}
			return count;
		}

		size_t GetUTF16StringLengthInBytes( void const * pSource_ )
		{
			size_t count = 0;
			char16_t const * ptr = reinterpret_cast< char16_t const * >( pSource_ );
			while ( *ptr++ )
			{
				count += 2;
				if ( ( *( ptr - 1 ) >= 0xD800 ) && ( *( ptr - 1 ) < 0xDC00 ) && *ptr && ( *ptr >= 0xDC00 ) && ( *ptr < 0xE000 ) )
				{
					++ptr;
					count += 2;
				}
			}
			return count;
		}

		size_t GetUTF32StringLengthInCharacters( void const * pSource_ )
		{
			char32_t const * ptr = reinterpret_cast< char32_t const * >( pSource_ );
			size_t length = 0;
			while ( *ptr++ )
				++length;
			return length;
		}

		size_t GetCodepointAndCountFromUTF8( char const * pSource_, char32_t & cpOut_ )
		{
			size_t count = 1;
			char32_t code = *pSource_++;
			{
				// 2 byte encoding?
				if ( ( code & 0xE0 ) == 0xC0 && ( *( pSource_ ) ) && ( *pSource_ & 0xC0 ) == 0x80 )
				{
					code = ( code & 0x1F ) << 6;
					code |= ( *pSource_++ & 0x3F );
					count = 2;
				}
				// 3 byte encoding?
				else if ( ( code & 0xF0 ) == 0xE0 && ( *pSource_ ) && ( *pSource_ & 0xC0 ) == 0x80 && ( *pSource_ + 1 ) && ( *( pSource_ + 1 ) & 0xC0 ) == 0x80 )
				{
					code = ( code & 0xF ) << 12;
					code |= ( *pSource_++ & 0x3F ) << 6;
					code |= ( *pSource_++ & 0x3F );
					count = 3;
				}
				// 4 byte encoding?
				else if ( ( code & 0xF8 ) == 0xF0 && ( *pSource_ ) && ( *pSource_ & 0xC0 ) == 0x80 && ( *( pSource_ + 1 ) ) && ( *( pSource_ + 1 ) & 0xC0 ) == 0x80 && ( *( pSource_ + 2 ) ) && ( *( pSource_ + 2 ) & 0xC0 ) == 0x80 )
				{
					code = ( code & 0x7 ) << 18;
					code |= ( *pSource_++ & 0x3F ) << 12;
					code |= ( *pSource_++ & 0x3F ) << 6;
					code |= ( *pSource_++ & 0x3F );
					count = 4;
				}
#if defined( STRICT_UTF8 )
				else if ( code & 0x80 )
					ASSERT_MSG( false, "Badly formed character found in UTF8" );
#endif // #if defined( STRICT_UTF8 )
			}
			cpOut_ = code;
			return count;
		}

		// note that this function returns the number of bytes in the encoding, not the number of 16bit characters
		size_t GetCodepointAndBytesFromUTF16( char16_t const * pSource_, char32_t & cpOut_ )
		{
			size_t bytes = 2;
			char32_t code = *pSource_++;
			{
				// 4 byte encoding?
				if ( ( code >= 0xD800 ) && ( code < 0xDC00 ) && *pSource_ && ( *pSource_ >= 0xDC00 ) && ( *pSource_ < 0xE000 ) )
				{
					code = ( code - 0xD800 ) << 10;
					code |= *pSource_ - 0xDC00;
					code += 0x10000;
					++pSource_;
					bytes = 4;
				}
			}
			cpOut_ = code;
			return bytes;
		}

		void GetUTF8FromCodePoint( char32_t const cp_, char * pOut_ )
		{
			if ( cp_ >= 0200 )
			{
				if ( cp_ >= 04000 )
				{
					if ( cp_ >= 0100000 )
					{
						// 4 byte encoding
						if ( cp_ >= 0200000 )
						{
							*pOut_++ = static_cast< char >( 0360 | ( cp_ >> 18 ) );
							*pOut_++ = static_cast< char >( 0200 | ( ( cp_ >> 12 ) & 077 ) );
							*pOut_++ = static_cast< char >( 0200 | ( ( cp_ >> 6 ) & 077 ) );
							*pOut_++ = static_cast< char >( 0200 | ( cp_ & 077 ) );
						}
						else
						{
							// 3 byte encoding
							*pOut_++ = static_cast< char >( 0350 | ( ( cp_ >> 12 ) & 07 ) );
							*pOut_++ = static_cast< char >( 0200 | ( ( cp_ >> 6 ) & 077 ) );
							*pOut_++ = static_cast< char >( 0200 | ( cp_ & 077 ) );
						}
					}
					else
					{
						// 3 byte encoding
						*pOut_++ = static_cast< char >( 0340 | ( cp_ >> 12 ) );
						*pOut_++ = static_cast< char >( 0200 | ( ( cp_ >> 6 ) & 077 ) );
						*pOut_++ = static_cast< char >( 0200 | ( cp_ & 077 ) );
					}
				}
				else
				{
					// 2 byte encoding
					*pOut_++ = static_cast< char >( 0300 | ( cp_ >> 6 ) );
					*pOut_++ = static_cast< char >( 0200 | ( cp_ & 077 ) );
				}
			}
			else
			{
				// single byte encoding
				*pOut_++ = static_cast< char >( cp_ );
			}
		}

		void GetUTF16FromCodePoint( char32_t const cp_, char16_t * pOut_ )
		{
			if ( cp_ < 0x10000 )
				*pOut_ = static_cast< char16_t >( cp_ );
			else
			{
				char32_t val = cp_ - 0x10000;
				*pOut_++ = static_cast< char16_t >( ( val >> 10 ) + 0xD800 );
				*pOut_++ = static_cast< char16_t >( ( val & 0x3FF ) + 0xDC00 );
			}
		}

		void UTF32ToUTF8( char32_t const * pSource_, char * pOut_ )
		{
			char32_t chr_;
			do
			{
				chr_ = *pSource_++;
				if ( chr_ >= 0200 )
				{
					if ( chr_ >= 04000 )
					{
						if ( chr_ >= 0100000 )
						{
							// 4 byte encoding
							if ( chr_ >= 0200000 )
							{
								*pOut_++ = static_cast< char >( 0360 | ( chr_ >> 18 ) );
								*pOut_++ = static_cast< char >( 0200 | ( ( chr_ >> 12 ) & 077 ) );
								*pOut_++ = static_cast< char >( 0200 | ( ( chr_ >> 6 ) & 077 ) );
								*pOut_++ = static_cast< char >( 0200 | ( chr_ & 077 ) );
							}
							else
							{
								// 3 byte encoding
								*pOut_++ = static_cast< char >( 0350 | ( ( chr_ >> 12 ) & 07 ) );
								*pOut_++ = static_cast< char >( 0200 | ( ( chr_ >> 6 ) & 077 ) );
								*pOut_++ = static_cast< char >( 0200 | ( chr_ & 077 ) );
							}
						}
						else
						{
							// 3 byte encoding
							*pOut_++ = static_cast< char >( 0340 | ( chr_ >> 12 ) );
							*pOut_++ = static_cast< char >( 0200 | ( ( chr_ >> 6 ) & 077 ) );
							*pOut_++ = static_cast< char >( 0200 | ( chr_ & 077 ) );
						}
					}
					else
					{
						// 2 byte encoding
						*pOut_++ = static_cast< char >( 0300 | ( chr_ >> 6 ) );
						*pOut_++ = static_cast< char >( 0200 | ( chr_ & 077 ) );
					}
				}
				else
				{
					// single byte encoding
					*pOut_++ = static_cast< char >( chr_ );
				}
			} while ( chr_ );
		}

		// returns the number of 8 bit characters required to hold the given UTF32 codepoint as UTF8
#if defined( __linux )
		size_t NumUTF8CharsFromCodepoint( wchar_t const cp_ )
#else
		size_t NumUTF8CharsFromCodepoint( char32_t const cp_ )
#endif
		{
			size_t count = 1;
			if ( cp_ >= 0200 )
			{
				++count;
				if ( cp_ >= 04000 )
				{
					++count;
					if ( cp_ >= 0200000 )
					{
						++count;
					}
				}
			}
			return count;
		}

		size_t NumUTF16CharsFromCodepoint( char32_t const cp_ )
		{
			size_t count = 1;
			if ( cp_ > 0xFFFF )
				count = 2;
			return count;
		}

#if defined( __linux )
		size_t GetUTF8RecodeLengthInBytes( wchar_t const * pSource_ )
#else
		size_t GetUTF8RecodeLengthInBytes( char32_t const * pSource_ )
#endif
		{
			size_t count = 0;
			while ( *pSource_ )
			{
				count += NumUTF8CharsFromCodepoint( *pSource_++ );
			}
			return count;
		}

		void UTF8ToUTF32( char const * pSource_, char32_t * pOut_ )
		{
			size_t total = 0;
			size_t count;
			char32_t code;

			while ( *pSource_ )
			{
				count = GetCodepointAndCountFromUTF8( pSource_, code );
				*pOut_++ = code;
				pSource_ += count;
			}
		}

		void UTF8ToUTF16( char const * pSource_, char16_t * pOut_ )
		{
			size_t total = 0;
			size_t count;
			char32_t code;

			while ( *pSource_ )
			{
				count = GetCodepointAndCountFromUTF8( pSource_, code );
				pSource_ += count;
				count = NumUTF16CharsFromCodepoint( code );
				GetUTF16FromCodePoint( code, pOut_ );
				pOut_ += count;
			}
		}
	}
}