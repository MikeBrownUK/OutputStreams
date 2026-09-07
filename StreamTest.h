//////////////////////////////////////////////////////////////////////////
/// Mike Brown, 2022-2026
///
/// Filename:	StreamTest.h
/// Created:	16/8/2022
/// Author:		Mike
/// 
/// Description: Test framework for OutputStreams
///				 Release 1.5
///
//////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef StreamTest_DEFINED_16_8_2022
#define StreamTest_DEFINED_16_8_2022

#include "gtest/gtest.h"
#include "OutputStreams/OutputChannels.h"

template< typename T_ >
using StreamList = std::vector < mbp::streams::BasicStream_t< T_ >* >;

using namespace mbp::streams;

auto constexpr kNarrowFilename = "outNarrow.test.txt";
auto constexpr kWideFilename = "outWide.test.txt";
auto constexpr kUTF8ReferenceFilename = "outUTF8_reference.test.txt";
auto constexpr kUTF8Filename = "outUTF8.test.txt";
auto constexpr kUTF32Filename = "outUTF32.test.txt";
auto constexpr kUTF16Filename = "outUTF16.test.txt";
auto constexpr kUTF16ReferenceFilename = "outUTF16_reference.test.txt";

// forward declare file cleanup
void CleanupFiles();

char const * kFilesToClean[] = {
		kNarrowFilename
	,	kWideFilename
	,	kUTF8ReferenceFilename
	,	kUTF8Filename
	,	kUTF32Filename
	,	kUTF16Filename
	,	kUTF16ReferenceFilename
};

class TestUsingFiles : public ::testing::Test
{
protected:
	~TestUsingFiles()
	{
		CleanupFiles();
	}
};

template< typename T_ >
class SingleChannelFile
{
public:
	SingleChannelFile( char const * filename_ )
		: m_stream( filename_ )
		, m_connector( StreamList< typename T_::elem >( { &m_stream } ) )
		, m_channel( 0, m_connector )
	{}
	StreamList< typename T_::elem > m_connector;
	OutputStreamComplete_t< OutputFile_t< typename T_::elem>, T_ > m_stream;
	OutputChannelComplete_t< T_ > m_channel;
};

class UTFChannelFiles : public TestUsingFiles
{
public:
	UTFChannelFiles()
		: m_fileUTF8( kUTF8Filename )
		, m_fileUTF8Ref( kUTF8ReferenceFilename )
#if defined( _MSC_VER )
		, m_fileUTF16( kUTF16Filename )
		, m_fileUTF16Ref( kUTF16ReferenceFilename )
#endif // #if defined( _MSC_VER )
	{}
	SingleChannelFile< Stream_t< char > > m_fileUTF8;
	SingleChannelFile< Stream_t< char > > m_fileUTF8Ref;
#if defined( _MSC_VER )
	SingleChannelFile< Stream_t< wchar_t > > m_fileUTF16;
	SingleChannelFile< Stream_t< wchar_t > > m_fileUTF16Ref;
#endif // #if defined( _MSC_VER )
};

class ThreadTester : public TestUsingFiles
{
protected:
	ThreadTester()
		: m_output( "", GetDefaultChannelSettings(), OutputStamp::GetDummyStamp() )
		, m_connector{ &m_output }
	{}
	OutputStreamComplete_t< OutputStdOut_t< char >, Stream_t< char > > m_output;
	StreamList< char > m_connector;
};

#endif // #ifndef StreamTest_DEFINED_16_8_2022