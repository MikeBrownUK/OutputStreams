//////////////////////////////////////////////////////////////////////////
/// Mike Brown, 2022
///
/// Filename:	StreamAndChannelAliases.h
/// Created:	7/7/2022
/// Author:		Mike
/// 
/// Description:
///
//////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef StreamAndChannelAliases_DEFINED_7_7_2022
#define StreamAndChannelAliases_DEFINED_7_7_2022

#include "OutputChannels.h"

namespace mbp
{
	namespace streams
	{
		//////////////////////////////////////////////////////////////////////////
		// OutputStream required and helpful aliases
		//////////////////////////////////////////////////////////////////////////
#if !defined( STREAM_OUTPUT_STRIP )
		// basic type aliases
		template< typename T_ >
		using BasicStream = BasicStream_t< T_ >;
		template< typename T_, template< typename > typename U_, template< typename > typename V_ = Stream_t >
		using OutputStream = OutputStream_t< T_, U_, V_ >;
		template< typename T_, template< typename > typename U_ = Stream_t >
		using OutputChannel = OutputChannel_t< T_, U_ >;
		template< typename T_, template< typename > typename U_, template< typename > typename V_ = Stream_t >
		using OutputStreamCapture = OutputStreamCapture_t< T_, U_, V_ >;
#define TLS_OUTPUTCHANNEL thread_local OutputChannel
		// helper aliases
		template< typename T_, template< typename > typename U_ = Stream_t >
		using StreamFile = OutputStream_t< T_, OutputFile_t, U_ >;
		template< typename T_, template< typename > typename U_ = Stream_t >
		using StreamStdOut = OutputStream_t< T_, OutputStdOut_t, U_ >;
#if defined (_MSC_VER)
		template< typename T_, template< typename > typename U_ = Stream_t >
		using StreamConsole = OutputStream_t< T_, OutputConsole_t, U_ >;
		template< typename T_, template< typename > typename U_ = Stream_t >
		using StreamDevStudio = OutputStream_t< T_, OutputDevStudio_t, U_ >;
#endif // #if defined (_MSC_VER)
		template< typename T_ >
		using StreamList = std::vector < BasicStream_t< T_ > * >;
#if defined( STREAM_TEST_SUITE )
		template< typename T_, template< typename > typename U_ = Stream_t >
		using StreamMem = OutputStream_t< T_, OutputMem_t, U_ >;
#endif // #if defined( STREAM_TEST_SUITE )

#else
		// basic types
		template< typename T_ >
		using BasicStream = NullStream_t< T_ >;
		template< typename T_, template< typename > typename U_, template< typename > typename V_ = Stream_t >
		using OutputStream = NullStream_t< T_ >;
		template< typename T_, template< typename > typename U_ = Stream_t >
		using OutputChannel = NullStream_t< T_ >;
		template< typename T_, template< typename > typename U_, template< typename > typename V_ = Stream_t >
		using OutputStreamCapture = NullStream_t< T_ >;
#define TLS_OUTPUTCHANNEL NullStream_t
		// helper types
		template< typename T_, template< typename > typename U_ = Stream_t >
		using StreamFile = NullStream_t< T_ >;
		template< typename T_, template< typename > typename U_ = Stream_t >
		using StreamStdOut = NullStream_t< T_ >;
		template< typename T_, template< typename > typename U_ = Stream_t >
		using StreamList = NullStream_t< T_ >;
		template< typename T_, template< typename > typename U_ = Stream_t >
		using OutputChannel = NullStream_t< T_ >;
#if defined (_MSC_VER)
		template< typename T_, template< typename > typename U_ = Stream_t >
		using StreamConsole = NullStream_t< T_ >;
		template< typename T_, template< typename > typename U_ = Stream_t >
		using StreamDevStudio = NullStream_t< T_ >;
#if defined( STREAM_TEST_SUITE )
		template< typename T_, template< typename > typename U_ = Stream_t >
		using StreamMem = NullStream_t< T_ >;
#endif // #if defined( STREAM_TEST_SUITE )
		
#endif // #if defined( _MSC_VER )
#endif // #if !defined( STREAM_OUTPUT_STRIP )
	}
}
#endif // #ifndefStreamAndChannelAliases_DEFINED_7_7_2022
