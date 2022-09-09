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
		template< typename T_, typename U_ >
		using OutputStream = OutputStream_t< Stream_t< T_ >, U_ >;
		template< typename T_, typename U_ >
		using OutputStreamConverting = OutputStream_t< ConvertingStream_t< T_ >, U_ >;
		template< typename T_ >
		using OutputChannel = OutputChannel_t< Stream_t< T_ > >;
		template< typename T_ >
		using OutputChannelConverting = OutputChannel_t< ConvertingStream_t< T_ > >;
		template< typename T_ >
		using Stream = Stream_t< T_ >;
		template< typename T_ >
		using ConvertingStream = ConvertingStream_t< T_ >;
		template< typename T_, typename U_ >
		using OutputStreamCapture = OutputStreamCapture_t< T_, U_ >;
#define TLS_OUTPUTCHANNEL thread_local OutputChannel
		// helper aliases
		template< typename T_ >
		using StreamFile = OutputStream_t< Stream_t< T_ >, OutputFile_t< T_ > >;
		template< typename T_ >
		using StreamStdOut = OutputStream_t< Stream_t< T_ >, OutputStdOut_t< T_ > >;
		template< typename T_ >
		using StreamFileConverting = OutputStream_t< ConvertingStream_t< T_ >, OutputFile_t< T_ > >;
		template< typename T_ >
		using StreamStdOutConverting = OutputStream_t< ConvertingStream_t< T_ >, OutputStdOut_t< T_ > >;
#if defined (_MSC_VER)
		template< typename T_ >
		using StreamConsole = OutputStream_t< Stream_t< T_ >, OutputConsole_t< T_ > >;
		template< typename T_ >
		using StreamDevStudio = OutputStream_t< Stream_t< T_ >, OutputDevStudio_t< T_ > >;
		template< typename T_ >
		using StreamConsoleConverting = OutputStream_t< ConvertingStream_t< T_ >, OutputConsole_t< T_ > >;
		template< typename T_ >
		using StreamDevStudioConverting = OutputStream_t< ConvertingStream_t< T_ >, OutputDevStudio_t< T_ > >;
#endif // #if defined (_MSC_VER)
		template< typename T_ >
		using StreamList = std::vector < BasicStream_t< T_ > * >;
#if defined( STREAM_TEST_SUITE )
		template< typename T_ >
		using StreamMem = OutputStream_t< Stream_t< T_ >, OutputMem_t< T_ > >;
		template< typename T_ >
		using StreamMemConverting = OutputStream_t< ConvertingStream_t< T_ >, OutputMem_t< T_ > >;
#endif // #if defined( STREAM_TEST_SUITE )

#else
		// basic types
		template< typename T_ >
		using BasicStream = NullStream_t< T_ >;
		template< typename T_ >
		using OutputStream = NullStream_t< T_ >;
		template< typename T_ >
		using OutputChannel = NullStream_t< T_ >;
		template< typename T_ >
		using Stream = NullStream_t< T_ >;
		template< typename T_ >
		using ConvertingStream = NullStream_t< T_ >;
		template< typename T_ >
		using OutputStreamCapture = NullStream_t< T_ >;
#define TLS_OUTPUTCHANNEL NullStream_t
		// helper types
		template< typename T_ >
		using StreamFile = NullStream_t< T_ >;
		template< typename T_ >
		using StreamStdOut = NullStream_t< T_ >;
		template< typename T_ >
		using StreamFileConverting = NullStream_t< T_ >;
		template< typename T_ >
		using StreamStdOutConverting = NullStream_t< T_ >;
		template< typename T_ >
		using StreamList = NullStream_t< T_ >;
		template< typename T_ >
		using OutputChannel = NullStream_t< T_ >;
		template< typename T_ >
		using OutputChannelConverting = NullStream_t< T_ >;
#if defined (_MSC_VER)
		template< typename T_ >
		using StreamConsole = NullStream_t< T_ >;
		template< typename T_ >
		using StreamDevStudio = NullStream_t< T_ >;
		template< typename T_ >
		using StreamConsoleConverting = NullStream_t< T_ >;
		template< typename T_ >
		using StreamDevStudioConverting = NullStream_t< T_ >;
#if defined( STREAM_TEST_SUITE )
		template< typename T_ >
		using StreamMem = NullStream_t< T_ >;
		template< typename T_ >
		using StreamMemConverting = NullStream_t< T_ >;
#endif // #if defined( STREAM_TEST_SUITE )
		
#endif // #if defined( _MSC_VER )
#endif // #if !defined( STREAM_OUTPUT_STRIP )
	}
}
#endif // #ifndefStreamAndChannelAliases_DEFINED_7_7_2022
