//////////////////////////////////////////////////////////////////////////
/// Mike Brown, 2022
///
/// Filename:	Strings.h
/// Created:	19/8/2022
/// Author:		Mike
/// 
/// Description: Various string manipulators
///
//////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef Strings_DEFINED_19_8_2022
#define Strings_DEFINED_19_8_2022

#include <cstddef>

namespace mbp
{
	namespace strings
	{
		// string encoding converters
		size_t GetUTF8StringLengthInBytes( void const * pSource_ );
		size_t GetUTF16StringLengthInBytes( void const * pSource_ );
		size_t GetUTF8StringLengthInCharacters( void const * pSource_ );
		size_t GetUTF16StringLengthInCharacters( void const * pSource_ );
		size_t GetUTF32StringLengthInCharacters( void const * pSource_ );
#if defined( __linux )
		size_t NumUTF8CharsFromCodepoint( wchar_t const cp_ );
#else
		size_t NumUTF8CharsFromCodepoint( char32_t const cp_ );
#endif // #if defined( __linux )
		size_t GetUTF8RecodeLengthInBytes( wchar_t const * pSource_ );
		size_t NumUTF16CharsFromCodepoint( char32_t const cp_ );

		// converts a UTF8 character into a UTF32 character (by reference) returning the number of bytes in the UTF8 encoding
		size_t GetCodepointAndCountFromUTF8( char const * pSource_, char32_t & cp_ );
		// converts a UTF16 character into a UTF32 character (by reference) returning the number of bytes in the UTF8 encoding
		size_t GetCodepointAndBytesFromUTF16( char16_t const * pSource_, char32_t & cp_ );
		// coverts a single UTF32 codepoint into a UTF8 sequence
		void GetUTF8FromCodePoint( char32_t const cp_, char * pOut_ );
		// coverts a single UTF32 codepoint into a UTF16 sequence
		void GetUTF16FromCodePoint( char32_t const cp_, char16_t * pOut_ );
		// convert a UTF8 string to a UTF32 string - call with second parameter nullptr to get character count
		void UTF8ToUTF32( char const * pSource_, char32_t * pOut_ );
		// convert a UTF32 sequence to UTF8, pass destination buffer pointer
		void UTF32ToUTF8( char32_t const * pSource_, char * pOut_ );
		// convert a UTF8 sequence to a UTF16 sequence
		void UTF8ToUTF16( char const * pSource_, char16_t * pOut_ );
	}
}

#endif // #ifndef Strings_DEFINED_19_8_2022
