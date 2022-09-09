//////////////////////////////////////////////////////////////////////////
/// ©Mike Brown Productions Limited, 2022
///
/// Filename:	StreamTest.cpp
/// Created:	16/8/2022
/// Author:		Mike
/// 
/// Description: Test framework for OutputStreams
///
//////////////////////////////////////////////////////////////////////////

#include "StreamTest.h"

#if defined(_MSC_VER)
#include <crtdbg.h>

#endif // #if defined(_MSC_VER)

#include "Utilities/Strings.h"
#include <chrono>
#include <random>
#include <thread>

using namespace mbp;
using namespace std::chrono;

//////////////////////////////////////////////////////////////////////////
/// Test Strings in UTF8
//	From https://www.kermitproject.org/utf8.html
//////////////////////////////////////////////////////////////////////////

const char * utf8StringOne =
u8R"(From the Anglo-Saxon Rune Poem (Rune version):
ᚠᛇᚻ᛫ᛒᛦᚦ᛫ᚠᚱᚩᚠᚢᚱ᛫ᚠᛁᚱᚪ᛫ᚷᛖᚻᚹᛦᛚᚳᚢᛗ
ᛋᚳᛖᚪᛚ᛫ᚦᛖᚪᚻ᛫ᛗᚪᚾᚾᚪ᛫ᚷᛖᚻᚹᛦᛚᚳ᛫ᛗᛁᚳᛚᚢᚾ᛫ᚻᛦᛏ᛫ᛞᚫᛚᚪᚾ
ᚷᛁᚠ᛫ᚻᛖ᛫ᚹᛁᛚᛖ᛫ᚠᚩᚱ᛫ᛞᚱᛁᚻᛏᚾᛖ᛫ᛞᚩᛗᛖᛋ᛫ᚻᛚᛇᛏᚪᚾ᛬)";
const char * utf8StringTwo =
u8R"(From Laȝamon's Brut (The Chronicles of England, Middle English, West Midlands, ca.1190):
An preost wes on leoden, Laȝamon was ihoten
He wes Leovenaðes sone -- liðe him be Drihten.
He wonede at Ernleȝe at æðelen are chirechen,
Uppen Sevarne staþe, sel þar him þuhte,
Onfest Radestone, þer he bock radde.
(The third letter in the author's name is Yogh, missing from many fonts; CLICK HERE for another Middle English sample with some explanation of letters and encoding).)";
const char * utf8StringThree =
u8R"(From the Tagelied of Wolfram von Eschenbach (Middle High German):
Sîne klâwen durh die wolken sint geslagen,
er stîget ûf mit grôzer kraft,
ich sih in grâwen tägelîch als er wil tagen,
den tac, der im geselleschaft
erwenden wil, dem werden man,
den ich mit sorgen în verliez.
ich bringe in hinnen, ob ich kan.
sîn vil manegiu tugent michz leisten hiez.)";
const char * utf8StringFour =
u8R"(Some lines of Odysseus Elytis (Greek):
Monotonic:
Τη γλώσσα μου έδωσαν ελληνική
το σπίτι φτωχικό στις αμμουδιές του Ομήρου.
Μονάχη έγνοια η γλώσσα μου στις αμμουδιές του Ομήρου.
από το Άξιον Εστί
του Οδυσσέα Ελύτη)";
const char * utf8StringFive =
u8R"(Polytonic:
Τὴ γλῶσσα μοῦ ἔδωσαν ἑλληνικὴ
τὸ σπίτι φτωχικὸ στὶς ἀμμουδιὲς τοῦ Ὁμήρου.
Μονάχη ἔγνοια ἡ γλῶσσα μου στὶς ἀμμουδιὲς τοῦ Ὁμήρου.
ἀπὸ τὸ Ἄξιον ἐστί
τοῦ Ὀδυσσέα Ἐλύτη)";
const char * utf8StringSix =
u8R"(The first stanza of Pushkin's Bronze Horseman (Russian):
На берегу пустынных волн
Стоял он, дум великих полн,
И вдаль глядел. Пред ним широко
Река неслася; бедный чёлн
По ней стремился одиноко.
По мшистым, топким берегам
Чернели избы здесь и там,
Приют убогого чухонца;
И лес, неведомый лучам
В тумане спрятанного солнца,
Кругом шумел.)";
const char * utf8StringSeven =
u8R"(Šota Rustaveli's Veṗxis Ṭq̇aosani, ̣︡Th, The Knight in the Tiger's Skin (Georgian):
ვეპხის ტყაოსანი შოთა რუსთაველი
ღმერთსი შემვედრე, ნუთუ კვლა დამხსნას სოფლისა შრომასა, ცეცხლს, წყალსა და მიწასა, ჰაერთა თანა მრომასა; მომცნეს ფრთენი და აღვფრინდე, მივჰხვდე მას ჩემსა ნდომასა, დღისით და ღამით ვჰხედვიდე მზისა ელვათა კრთომაასა.)";
const char * utf8StringEight =
u8R"(Tamil poetry of Subramaniya Bharathiyar: சுப்ரமணிய பாரதியார் (1882-1921):
யாமறிந்த மொழிகளிலே தமிழ்மொழி போல் இனிதாவது எங்கும் காணோம்,
பாமரராய் விலங்குகளாய், உலகனைத்தும் இகழ்ச்சிசொலப் பான்மை கெட்டு,
நாமமது தமிழரெனக் கொண்டு இங்கு வாழ்ந்திடுதல் நன்றோ? சொல்லீர்!
தேமதுரத் தமிழோசை உலகமெலாம் பரவும்வகை செய்தல் வேண்டும்.)";
const char * utf8StringNine =
u8R"(Kannada poetry by Kuvempu — ಬಾ ಇಲ್ಲಿ ಸಂಭವಿಸು
ಬಾ ಇಲ್ಲಿ ಸಂಭವಿಸು ಇಂದೆನ್ನ ಹೃದಯದಲಿ
ನಿತ್ಯವೂ ಅವತರಿಪ ಸತ್ಯಾವತಾರ
ಮಣ್ಣಾಗಿ ಮರವಾಗಿ ಮಿಗವಾಗಿ ಕಗವಾಗೀ...
ಮಣ್ಣಾಗಿ ಮರವಾಗಿ ಮಿಗವಾಗಿ ಕಗವಾಗಿ
ಭವ ಭವದಿ ಭತಿಸಿಹೇ ಭವತಿ ದೂರ
ನಿತ್ಯವೂ ಅವತರಿಪ ಸತ್ಯಾವತಾರ || ಬಾ ಇಲ್ಲಿ ||)";

const char * AllUTF8Strings[] =
{
		utf8StringOne
	,	utf8StringTwo
	,	utf8StringThree
	,	utf8StringFour
	,	utf8StringFive
	,	utf8StringSix
	,	utf8StringSeven
	,	utf8StringEight
	,	utf8StringNine
};

// Whilst channel IDs are plain ints internally, enums make channel intent a little more obvious. They are meaningless within the scope of this test program, just examples of possible use.

namespace ChannelEnums
{
	enum Channels
	{
			// Channel IDs must be within 0 ... (mbp::streams::kMaxOutputChannels - 1)
			DEFAULT = 0
			, USER_INTERFACE
			, NETWORK_LAYER
			// etc...
			, INVALID_CHANNEL = streams::kMaxOutputChannels		// beyond the end of the array
	};
}

using namespace ChannelEnums;

void CleanupFiles()
{
#if defined( _MSC_VER )
	for( auto i : kFilesToClean )
		DeleteFileA( i );
#endif
}

void WriteUTF8CodeRange( BasicStream_t< char > & out, size_t firstCP_, size_t lastCP_, size_t glyphsPerRow_, bool header_ = true, bool spaces_ = true, bool skipControls = true )
{
	char buffer[ 5 ];
	size_t charsPerGlyph;
	size_t count = 0;

	if ( header_ )
		out << std::hex << std::setw( 6 ) << std::setfill( '0' ) << "UTF characters from codepoint 0x" << firstCP_ << " to 0x" << lastCP_ << ":" << endl << std::hex << std::setw( 6 ) << std::setfill( '0' ) << ( firstCP_ / glyphsPerRow_ ) * glyphsPerRow_ << "\t";

	for ( auto i = firstCP_; i <= lastCP_; ++i )
	{
		charsPerGlyph = strings::NumUTF8CharsFromCodepoint( static_cast< char32_t >( i ) );
		strings::GetUTF8FromCodePoint( static_cast< char32_t >( i ), buffer );
		buffer[ charsPerGlyph ] = 0;
		if ( ( skipControls ) && ( i < 0x80 || i > 0x9F ) )
			out << buffer;
		if ( spaces_ )
			out << " ";
		if ( i == 0xD7FF )	// skip surrogates section
			i = 0xDFFF;
		if ( ++count == glyphsPerRow_ && ( i < lastCP_ ) )
		{
			count = 0;
			out << endl << std::setw( 6 ) << std::setfill( '0' ) << std::hex << ( ( i + 1 ) / glyphsPerRow_ ) * glyphsPerRow_ << "\t";
		}
	}
	out.flush();
}

void WriteUTF16CodeRange( BasicStream_t< wchar_t > & out, size_t firstCP_, size_t lastCP_, size_t glyphsPerRow_, bool header_ = true, bool spaces_ = true, bool skipControls = true )
{
	wchar_t buffer[ 3 ];
	size_t charsPerGlyph;
	size_t count = 0;

	if ( header_ )
		out << std::hex << std::setw( 6 ) << std::setfill( L'0' ) << L"UTF characters from codepoint 0x" << firstCP_ << L" to 0x" << lastCP_ << L":" << endl << std::hex << std::setw( 6 ) << std::setfill( L'0' ) << ( firstCP_ / glyphsPerRow_ ) * glyphsPerRow_ << L"\t";

	for ( auto i = firstCP_; i <= lastCP_; ++i )
	{
		charsPerGlyph = strings::NumUTF16CharsFromCodepoint( static_cast< char32_t >( i ) );
		strings::GetUTF16FromCodePoint( static_cast< char32_t >( i ), reinterpret_cast< char16_t * >( buffer ) );
		buffer[ charsPerGlyph ] = 0;
		if ( ( skipControls ) && ( i < 0x80 || i > 0x9F ) )
			out << buffer;
		if ( spaces_ )
			out << L" ";
		if ( i == 0xD7FF )	// skip surrogates section
			i = 0xDFFF;
		++count;
		if ( count == glyphsPerRow_ && ( i < lastCP_ ) )
		{
			count = 0;
			out << endl << std::setw( 6 ) << std::setfill( L'0' ) << std::hex << ( ( i + 1 ) / glyphsPerRow_ ) * glyphsPerRow_ << L"\t";
		}
	}
	out.flush();
}

bool isValidUTF8( char const * pBuff_, size_t * numBytes_ = nullptr )
{
	auto setNumBytes = [&]( size_t num_ ) { if ( numBytes_ ) *numBytes_ = num_; };

	bool good = false;
	unsigned char const * pUnsigned = reinterpret_cast< unsigned char const * >( pBuff_ );
	unsigned char code;
	setNumBytes( 0 );
	if ( *pUnsigned & 0x80 )
	{
		code = *pUnsigned & 0370;
		if ( code == 0360 )
		{
			// check legal 4 byte encoding
			if ( *pUnsigned + 1 && ( *( pUnsigned + 1 ) & 0300 ) == 0200 && *pUnsigned + 2 && ( *( pUnsigned + 2 ) & 0300 ) == 0200 && *pUnsigned + 3 && ( *( pUnsigned + 3 ) & 0300 ) == 0200 )
			{
				setNumBytes( 4 );
				good = true;
			}
		}
		else if ( code == 0350 || code == 0340 )
		{
			// check legal 3 byte encoding
			if ( *pUnsigned + 1 && ( *( pUnsigned + 1 ) & 0300 ) == 0200 && *pUnsigned + 2 && ( *( pUnsigned + 2 ) & 0300 ) == 0200 )
			{
				setNumBytes( 3 );
				good = true;
			}
		}
		else
		{
			// check 2 byte encoding
			if ( ( *pUnsigned & 0300 ) == 0300 )
			{
				if ( *( pUnsigned + 1 ) && ( *( pUnsigned + 1 ) & 0300 ) == 0200 )
				{
					setNumBytes( 2 );
					good = true;
				}
			}
		}
	}
	else
	{
		setNumBytes( 1 );
		good = true;
	}
	return good;
}

bool isValidUTF16( wchar_t const * pBuff_, size_t * numBytes_ = nullptr )
{
	auto setNumBytes = [&]( size_t num_ ) { if ( numBytes_ ) *numBytes_ = num_; };

	bool good = false;
	setNumBytes( 0 );
	if ( *pBuff_ && ( ( *pBuff_ >= 0xD800 ) && ( *pBuff_ < 0xDC00 ) ) )
	{
		if( ( *pBuff_ + 1 ) && ( ( *( pBuff_ + 1 ) >= 0xDC00 ) && ( *( pBuff_ + 1 ) < 0xE000 ) ) )
		{
			setNumBytes( 4 );
			good = true;
		}
	}
	else
	{
		// single word
		setNumBytes( 2 );
		good = true;
	}
	return good;
}

TEST( SelfTests, CheckUTF8Validator )
{
	char buffer[ 4 ]{ 0,0,0,0 };

	bool allPassed;
	
	// test fail single character > 0x7F
	buffer[ 0 ] = char( 0xa1 );
	allPassed = ( false == isValidUTF8( buffer ) );
	// test pass single character
	buffer[ 0 ] = char( 0x50 );
	allPassed &= isValidUTF8( buffer );
	// test fail double byte sequence, legal first byte for two byte encoding, illegal second
	buffer[ 0 ] = char( 0320 );
	buffer[ 1 ] = char( 0302 );
	allPassed &= ( false == isValidUTF8( buffer ) );
	// test pass double byte
	buffer[ 0 ] = char( 0xc2 );
	buffer[ 1 ] = char( 0xa1 );
	allPassed &= isValidUTF8( buffer );
	// test fail three byte sequence
	buffer[ 0 ] = char( 0346 );	// legal
	buffer[ 1 ] = char( 0255 ); // legal
	buffer[ 2 ] = char( 0100 ); // fail
	allPassed &= ( false == isValidUTF8( buffer ) );
	// test pass three bytes
	buffer[ 2 ] = char( 0240 );
	allPassed &= isValidUTF8( buffer );
	// test fail four bytes, four ways
	buffer[ 0 ] = char( 0366 );	// legal
	buffer[ 1 ] = char( 0255 ); // legal
	buffer[ 2 ] = char( 0100 ); // fail
	buffer[ 3 ] = char( 0201 ); // legal
	allPassed &= ( false == isValidUTF8( buffer ) );
	buffer[ 2 ] = char( 0240 );
	buffer[ 1 ] = char( 0140 ); // fail
	allPassed &= ( false == isValidUTF8( buffer ) );
	buffer[ 1 ] = char( 0255 );
	buffer[ 3 ] = char( 030 ); // fail
	allPassed &= ( false == isValidUTF8( buffer ) );
	// test pass four bytes
	buffer[ 3 ] = char( 0220 );
	allPassed &= isValidUTF8( buffer );
	EXPECT_EQ( allPassed, true );
}

TEST( InitAndCleanupTests, CheckStreamCleanup_Single )
{
#if defined( _MSC_VER )
	_CrtMemState entryState, exitState, diffState;
	_CrtMemCheckpoint( &entryState );
#endif // #if defined( _MSC_VER )
	{
		OutputStream_t< Stream_t< char >, OutputStdOut_t< char > > output;
		output << "Single Stream cleanup test... Some numbers: " << 18 << ", " << std::hex << 65535 << ", " << 3.4 << ", " << 5.1f << endl;
	}
#if defined( _MSC_VER )
	_CrtMemCheckpoint( &exitState );
	EXPECT_EQ( 0, _CrtMemDifference( &diffState, &entryState, &exitState ) );
	CleanupFiles();
#endif
}

TEST( InitAndCleanupTests, CheckChannelCleanup_Single )
{
#if defined( _MSC_VER )
#if defined( _DEBUG )
	_CrtMemState entryState, exitState, diffState;
#endif
	_CrtMemCheckpoint( &entryState );
#endif // #if defined( _MSC_VER )
	{
		OutputStream_t< Stream_t< char >, OutputStdOut_t< char > > output;
		StreamList< char > connector{ &output };
		OutputChannel_t< Stream_t< char > > channel( 0, connector, false );

		channel << "Single Channel cleanup test... Some more numbers: " << 41 << ", " << std::oct << 9 << ", " << 4537 << endl;
	}
#if defined( _MSC_VER )
	_CrtMemCheckpoint( &exitState );
	EXPECT_EQ( 0, _CrtMemDifference( &diffState, &entryState, &exitState ) );
	CleanupFiles();
#endif
}

TEST( InitAndCleanupTests, CheckMany )
{
#if defined( _MSC_VER )
	_CrtMemState entryState, exitState, diffState;
	_CrtMemCheckpoint( &entryState );
#endif // #if defined( _MSC_VER )
	{
		OutputStream< char, OutputStdOut_t< char > > outCharSingle;
		StreamList< char > connectorSingle{ &outCharSingle };
		OutputChannel<  char > singleThreadChannel( 0, connectorSingle, false );
		OutputStream<  char32_t, OutputFile_t< char32_t > > out32File( kUTF32Filename );
		OutputStreamConverting< char32_t , OutputStdOut_t< char32_t > > out32StdOut;
		StreamList< char32_t > connectorMulti{ &out32File, &out32StdOut };
		OutputChannelConverting< char32_t > multiThreadMultiOut( 1, connectorMulti, true, SystemTimeStamp_t< char32_t >::GetInstance() );

		outCharSingle << "Multi Channel cleanup test ... " << 4.965 << ", " << 16384 << endl;
		multiThreadMultiOut << U"UTF32 test" << endl;
	}
#if defined( _MSC_VER )
	_CrtMemCheckpoint( &exitState );
	EXPECT_EQ( 0, _CrtMemDifference( &diffState, &entryState, &exitState ) );
	CleanupFiles();
#endif
}

TEST_F( UTFChannelFiles, CheckUTF8EncodingsViaFile )
{
	auto constexpr kFirstCP = 0x20;
	auto constexpr kLastCP = 0x10100;	// check many codes, just into the four byte region
	auto constexpr kGlyphsPerRow = 32;

	bool allOK;

	// write the encodings to the default UTF8 file stream object
	WriteUTF8CodeRange( m_fileUTF8.m_channel, kFirstCP, kLastCP, kGlyphsPerRow );
	// read the file back into memory
	size_t fileSize = 0;
	size_t bytesChecked;
	size_t count;
	char * utfPtr;

	std::ifstream inFile( kUTF8Filename, std::ios::binary );
	allOK = inFile.good();
	
	if ( allOK )
	{
		inFile.seekg( 0, std::ios_base::end );
		if ( inFile.good() )
			fileSize = static_cast< size_t >( inFile.tellg() );
		
		char * buffer = reinterpret_cast< char * >( malloc( fileSize ) );
		inFile.seekg( 0, std::ios_base::beg );
		inFile.read( buffer, fileSize );
		allOK = inFile.good();
		if ( allOK )
		{
			utfPtr = buffer;
			for ( bytesChecked = 0; ( bytesChecked < fileSize ) && allOK; )
			{
				allOK &= isValidUTF8( utfPtr, &count );
				utfPtr += count;
				bytesChecked += count;
			}
		}
		if ( buffer )
			free( buffer );
	}
	EXPECT_EQ( allOK, true );
}

// Microsoft only UTF16 test using wchar_t stream
#if defined( _MSC_VER )
TEST_F( UTFChannelFiles, CheckUTF16EncodingsViaFile )
{
	auto constexpr kFirstCP = 0x20;
	auto constexpr kLastCP = 0x10100;	// check many codes, just into the two word region
	auto constexpr kGlyphsPerRow = 32;

	bool allOK;

	// write the encodings to the default UTF8 file stream object
	WriteUTF16CodeRange( m_fileUTF16.m_channel, kFirstCP, kLastCP, kGlyphsPerRow );
	// read the file back into memory
	size_t fileSize = 0;
	size_t bytesChecked;
	size_t count;
	wchar_t * utfPtr;

	std::ifstream inFile( kUTF16Filename, std::ios_base::in | std::ios_base::binary );
	allOK = inFile.good();

	if ( allOK )
	{
		inFile.seekg( 0, std::ios_base::end );
		if ( inFile.good() )
			fileSize = static_cast< size_t >( inFile.tellg() );

		char * fileBuffer = reinterpret_cast< char * >( malloc( fileSize ) );
		inFile.seekg( 0, std::ios_base::beg );
		inFile.read( fileBuffer, fileSize );
		allOK = inFile.good();
		if ( allOK )
		{
			utfPtr = reinterpret_cast< wchar_t * >( fileBuffer );
			for ( bytesChecked = 0; ( bytesChecked < fileSize ) && allOK; )
			{
				allOK &= isValidUTF16( utfPtr, &count );
				utfPtr += count / sizeof( wchar_t );
				bytesChecked += count;
			}
		}
		if ( fileBuffer )
			free( fileBuffer );
	}
	EXPECT_EQ( allOK, true );
}
#endif // #if defined ( _MSC_VER )

// Check UTF8 to UTF16 conversion and back again - MSC only
#if defined( _MSC_VER )
TEST_F( UTFChannelFiles, CheckUTF8ToUTF16Conversion )
{
	auto constexpr kFirstCP = 0x20;
	auto constexpr kLastCP = 0x10100;
	auto constexpr kGlyphsPerRow = 64;

	bool allOK;

	// write the reference encodings for UTF8 and UTF16
	WriteUTF8CodeRange( m_fileUTF8Ref.m_channel, kFirstCP, kLastCP, kGlyphsPerRow );
	WriteUTF16CodeRange( m_fileUTF16Ref.m_channel, kFirstCP, kLastCP, kGlyphsPerRow );

	// read the UTF8 file back into memory, convert into UTF16 and write back to temp UTF16 file
	size_t fileSizeOne = 0;
	size_t fileSizeTwo = 0;
	char * fileBufferOne;
	char * fileBufferTwo;
	size_t bytesProcessed;
	size_t count;
	char32_t cp;
	const char * utfPtr;
	wchar_t buffer[ 3 ];

	std::ifstream inFileOne( kUTF8ReferenceFilename, std::ios::binary );
	std::ifstream inFileTwo;

	allOK = inFileOne.good();

	if ( allOK )
	{
		inFileOne.seekg( 0, std::ios_base::end );
		if ( inFileOne.good() )
			fileSizeOne = static_cast< size_t >( inFileOne.tellg() );

		fileBufferOne = reinterpret_cast< char * >( malloc( fileSizeOne ) );
		inFileOne.seekg( 0, std::ios_base::beg );
		inFileOne.read(  fileBufferOne , fileSizeOne );
		utfPtr = fileBufferOne;
		allOK = inFileOne.good();
		if ( allOK )
		{
			for ( bytesProcessed = 0; ( bytesProcessed < fileSizeOne ) && allOK; )
			{
				count = strings::GetCodepointAndCountFromUTF8( utfPtr, cp );
				strings::GetUTF16FromCodePoint( cp, reinterpret_cast< char16_t * >( buffer ) );
				buffer[ strings::NumUTF16CharsFromCodepoint( cp ) ] = 0;
				m_fileUTF16.m_channel << buffer;
				utfPtr += count;
				bytesProcessed += count;
			}
			m_fileUTF16.m_channel.flush();
		}
		if ( fileBufferOne )
			free( fileBufferOne );

		// read in and compare the two UTF16 files
		inFileOne.close();
		inFileOne.open( kUTF16ReferenceFilename, std::ios_base::beg | std::ios_base::binary );
		inFileTwo.open( kUTF16Filename, std::ios_base::beg | std::ios_base::binary );

		inFileOne.seekg( 0, std::ios_base::end );
		if ( inFileOne.good() )
			fileSizeOne = static_cast< size_t >( inFileOne.tellg() );

		inFileTwo.seekg( 0, std::ios_base::end );
		if ( inFileTwo.good() )
			fileSizeTwo = static_cast< size_t >( inFileTwo.tellg() );

		allOK = fileSizeOne == fileSizeTwo && inFileOne.good() && inFileTwo.good();
		if ( allOK )
		{
			inFileOne.seekg( 0, std::ios_base::beg );
			inFileTwo.seekg( 0, std::ios_base::beg );
			fileBufferOne = reinterpret_cast< char * >( malloc( fileSizeOne ) );
			fileBufferTwo = reinterpret_cast< char * >( malloc( fileSizeOne ) );
			inFileOne.read( fileBufferOne, fileSizeOne );
			inFileTwo.read( fileBufferTwo, fileSizeOne );
			allOK = 0 == memcmp( fileBufferOne, fileBufferTwo, fileSizeOne );
			if ( !allOK )
				std::cout << "UTF8->UTF16 conversion does not match UTF16 reference file" << std::endl;
			if ( fileBufferOne )
				free( fileBufferOne );
			if ( fileBufferTwo )
				free( fileBufferTwo );
		}
	}
	EXPECT_EQ( allOK, true );
}
#endif // #if defined( _MSC_VER )

// Check UTF16 to UTF8 conversion and back again - again, MSC only
#if defined( _MSC_VER )
TEST_F( UTFChannelFiles, CheckUTF16ToUTF8Conversion )
{
	auto constexpr kFirstCP = 0x20;
	auto constexpr kLastCP = 0x10100;
	auto constexpr kGlyphsPerRow = 64;

	bool allOK;

	// write the reference encodings for UTF8 and UTF16
	WriteUTF8CodeRange( m_fileUTF8Ref.m_channel, kFirstCP, kLastCP, kGlyphsPerRow );
	WriteUTF16CodeRange( m_fileUTF16Ref.m_channel, kFirstCP, kLastCP, kGlyphsPerRow );

	// read the UTF16 file back into memory, convert into UTF8 and write back to temp UTF8 file
	size_t fileSizeOne = 0;
	size_t fileSizeTwo = 0;
	char * fileBufferOne;
	char * fileBufferTwo;
	size_t bytesProcessed;
	size_t count;
	char32_t cp;
	const char16_t * utfPtr;
	char buffer[ 5 ];

	std::ifstream inFileOne( kUTF16ReferenceFilename, std::ios_base::in | std::ios::binary );
	std::ifstream inFileTwo;

	allOK = inFileOne.good();

	if ( allOK )
	{
		inFileOne.seekg( 0, std::ios_base::end );
		if ( inFileOne.good() )
			fileSizeOne = static_cast< size_t >( inFileOne.tellg() );

		fileBufferOne = reinterpret_cast< char * >( malloc( fileSizeOne ) );
		inFileOne.seekg( 0, std::ios_base::beg );
		inFileOne.read( fileBufferOne, fileSizeOne );
		utfPtr = reinterpret_cast< char16_t * >( fileBufferOne );
		allOK = inFileOne.good();
		if ( allOK )
		{
			for ( bytesProcessed = 0; ( bytesProcessed < fileSizeOne ) && allOK; )
			{
				count = strings::GetCodepointAndBytesFromUTF16( utfPtr, cp );
				strings::GetUTF8FromCodePoint( cp, buffer );
				buffer[ strings::NumUTF8CharsFromCodepoint( cp ) ] = 0;
				m_fileUTF8.m_channel << buffer;
				utfPtr += count / sizeof( char16_t );
				bytesProcessed += count;
			}
			m_fileUTF8.m_channel.flush();
		}
		if ( fileBufferOne )
			free( fileBufferOne );

		// read in and compare the two UTF16 files
		inFileOne.close();
		inFileOne.open( kUTF8ReferenceFilename, std::ios_base::beg | std::ios_base::in | std::ios_base::binary );
		inFileTwo.open( kUTF8Filename, std::ios_base::beg | std::ios_base::in | std::ios_base::binary );

		inFileOne.seekg( 0, std::ios_base::end );
		if ( inFileOne.good() )
			fileSizeOne = static_cast< size_t >( inFileOne.tellg() );

		inFileTwo.seekg( 0, std::ios_base::end );
		if ( inFileTwo.good() )
			fileSizeTwo = static_cast< size_t >( inFileTwo.tellg() );

		allOK = fileSizeOne == fileSizeTwo && inFileOne.good() && inFileTwo.good();
		if ( allOK )
		{
			inFileOne.seekg( 0, std::ios_base::beg );
			inFileTwo.seekg( 0, std::ios_base::beg );
			fileBufferOne = reinterpret_cast< char * >( malloc( fileSizeOne ) );
			fileBufferTwo = reinterpret_cast< char * >( malloc( fileSizeTwo ) );
			inFileOne.read( fileBufferOne, fileSizeOne );
			inFileTwo.read( fileBufferTwo, fileSizeTwo );
			allOK = 0 == memcmp( fileBufferOne, fileBufferTwo, fileSizeOne );
			if ( !allOK )
				std::cout << "UTF16->UTF8 conversion does not match UTF16 reference file" << std::endl;
			if ( fileBufferOne )
				free( fileBufferOne );
			if ( fileBufferTwo )
				free( fileBufferTwo );
		}
	}
	EXPECT_EQ( allOK, true );
}

#endif // #if defined ( _MSC_VER )

void EmptyThreadFunc( int milliSecondsToWait, StreamList< char >const & connector_ )
{
	OutputChannel< char > channel( DEFAULT, connector_, true );

	system_clock::time_point start = system_clock::now();
	system_clock::time_point wait = start + std::chrono::milliseconds( milliSecondsToWait );
	while ( system_clock::now() <= wait )
	{
		//...
	}
	channel << "Worker thread writing to channel after settings altered by other thread. SHOULD NOT display" << endl;
}

// check settings are consistent after delayed thread start and that thread reference count is as expected
TEST_F( ThreadTester, TestMultithreadInitialisation )
{
	StreamSettings * settingsReference = g_AllChannelSettings;
	uint8_t * flagsReference = g_channelInitFlags;

	SettingsType modifiedEnable = 0;
	SettingsType modifiedDefaultPriority = 6;
	SettingsType modifiedFilter = 5;

	StreamSettings nonStandardSettings = { modifiedEnable, modifiedDefaultPriority, modifiedFilter };

	OutputChannel< char > channel( DEFAULT, m_connector, true );

	system_clock::time_point start = system_clock::now();
	system_clock::time_point wait = start + system_clock::duration( std::chrono::seconds( 1 ) );

	// inject new settings into the channel
	channel << "This line should display on std::out" << endl;
	channel << Enable( modifiedEnable ) << DefaultPriority( modifiedDefaultPriority ) << Filter( modifiedFilter ) << endl;
	channel << "This line SHOULD NOT display" << endl;

	while( system_clock::now() <= wait )
	{
		channel << "Waiting on timer..." << endl;
		std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	}
	std::thread workerThreadOne( EmptyThreadFunc, 1000, m_connector );
	std::thread workerThreadTwo( EmptyThreadFunc, 800, m_connector );
	workerThreadTwo.join();
	g_streamsMutex.lock();
	bool allOK = 3 > g_channelInitFlags[ 0 ];
	g_streamsMutex.unlock();
	workerThreadOne.join();
	g_streamsMutex.lock();
	allOK &= 1 == g_channelInitFlags[ 0 ];
	g_streamsMutex.unlock();

	SettingsType retrievedEnable = channel.GetEnable();
	SettingsType retrievedPriority = channel.GetPriority();
	SettingsType retrievedFilter = channel.GetFilter();
	
	allOK &= ( retrievedEnable == modifiedEnable )
		&& ( retrievedPriority == modifiedDefaultPriority )
		&& ( retrievedFilter == modifiedFilter );

	EXPECT_EQ( allOK, true );
}

// Test that OutputStamps work correctly, before Channel creation, during their use and after their detachment
TEST( GeneralTests, CheckOutputStamps )
{
	bool allOK;
	uint8_t * flagsReference = g_channelInitFlags;

	StreamStdOut< char > streamOne( nullptr, SystemTimeStamp_t<char>::GetInstance() );
	StreamList< char > connectorOne{ &streamOne };
	streamOne << "Stream One writing to std::out before channels attached" << endl;
	{
		OutputChannel< char > channelOne( DEFAULT, connectorOne, false, SystemTimeStamp_t<char>::GetInstance() );
		OutputChannel< char > channelTwo( DEFAULT, connectorOne, false );
		channelOne << "Channel One writing to std::out via Stream One using its own Timestamp" << endl;
		streamOne << "Writing to Stream One directly with channels in use. This is safe in single thread mode. Timestamp will not appear though, as stream is currently flagged as in use by OutputChannels." << endl;
		channelTwo << "Channel Two is a proxy using same channel ID and connector so I don't have to set up threads but with no Timestamp, also using Stream One" << endl;
		channelOne << "Another line on Channel One" << endl;
		std::streampos streamPosition = streamOne.tellp();
		std::streampos c1Position = channelOne.tellp();
		std::streampos c2Position = channelTwo.tellp();
		allOK = static_cast< size_t >( streamPosition ) == 0 && c1Position > 0 && static_cast< size_t >( c2Position ) == 0;
		allOK &= true == streamOne.GetIsChannelTarget();
	}
	streamOne << "Stream One writing to std::out following detachment of both channels. Its Timestamp, if used, is also restored at this point." << endl;
	std::streampos streamPosition = streamOne.tellp();
	allOK &= streamPosition > 0;
	allOK &= false == streamOne.GetIsChannelTarget();
}

void SameIDThreadFunc( int threadNumber_, int channelNumber_, StreamList< char > const & connector_, OutputStamp&& stamp_, size_t msToRun_ )
{
	OutputChannel< char > channel( channelNumber_, connector_, true , stamp_ );

	system_clock::time_point start = system_clock::now();
	system_clock::time_point wait = start + std::chrono::milliseconds( msToRun_ );
	while ( system_clock::now() <= wait )
	{
		channel << "Thread " << threadNumber_ << " writing to channel " << channelNumber_ << endl;
	}
}

TEST( GeneralTests, SameChannelIDsDifferentParams )
{
	bool allOK = true;
	uint8_t * flagsReference = g_channelInitFlags;

	// define three different streams
	StreamStdOut< char > streamOne( nullptr, SystemTimeStamp_t<char>::GetInstance() );
	// this one uses the LineStamp example class just to test a variable length stamp prefix
	StreamStdOut< char > streamTwo( nullptr, LineStamp_t< char >::GetInstance() );
	StreamFile< char > streamThree( kNarrowFilename, SystemTimeStamp_t<char>::GetInstance() );
	
	// define two connectors to link the streams to channels. Both go to std::out via different streams, but one additionally sends to a file 
	StreamList< char > connectorOne{ &streamOne };	
	StreamList< char > connectorTwo{ &streamTwo, &streamThree };

	// test streams and channels are in expected state following each set of tests
	auto cleanupVerify = [&]()
	{
		// the two streams should have none zeroed put positions following all channel disconnects as both use OutputStamps
		std::streampos streamPos1 = streamOne.tellp();
		std::streampos streamPos2 = streamTwo.tellp();
		allOK &= streamPos1 == SystemTimeStamp_t<char>::GetInstance().GetMaxLength();
		allOK &= streamPos2 == LineStamp_t<char>::GetInstance().GetMaxLength();
		// neither should think it has a channel attached...
		allOK &= false == streamOne.GetIsChannelTarget();
		allOK &= false == streamTwo.GetIsChannelTarget();
		// ensure no channels are marked as existing in the channel reference counters
		g_streamsMutex.lock();
		for ( auto i : g_channelInitFlags )
		{
			allOK &= i == 0;
		}
		g_streamsMutex.unlock();
	};

	{
		// create two channels using same channel ID but writing to different streams via their connectors
		// The expected behaviour here is that they share channel filter flags but do not otherwise interfere with each other.
		OutputChannel< char > channelOne( DEFAULT, connectorOne, false, SystemTimeStamp_t<char>::GetInstance() );
		// same ID, different stream list, though both ultimately go to std::out by design
		OutputChannel< char > channelTwo( DEFAULT, connectorTwo, false, SystemTimeStamp_t<char>::GetInstance() );
		OutputChannel< char > channelThree( DEFAULT, connectorOne, false, LineStamp_t< char >::GetInstance() );
		OutputChannel< char > channelFour( DEFAULT, connectorTwo, false, LineStamp_t< char >::GetInstance() );
		channelOne << "Channel One..." << endl;
		channelTwo << "Channel Two, different stream but same ultimate target" << endl;
		// filter out everything
		channelOne << Filter( 0 ) << "Channel One - SHOULD NOT display" << endl;
		channelOne << "Channel Two - SHOULD also NOT display" << endl;
		// reset filter to let everything through
		channelOne << Filter( ~0 );
	}

	cleanupVerify();

	// #TODO - these tests should go through a pipe to work out if the output is correct rather than rely on visuals for some aspects
	streamOne << "Stream One after channel disconnect..." << endl;
	streamTwo << "Stream Two after channel disconnect..." << endl;

	// now create six worker threads that use one of the two stream connectors we defined earlier, use differing OutputStamps and have them spew to their channel objects
	std::thread workerThreadOne( SameIDThreadFunc, 1, 0, connectorOne, SystemTimeStamp_t<char>::GetInstance(), 500 );
	std::thread workerThreadTwo( SameIDThreadFunc, 2, 1, connectorTwo, OutputStamp::GetDummyStamp(), 500 );

	std::thread workerThreadThree( SameIDThreadFunc, 3, 0, connectorOne, OutputStamp::GetDummyStamp(), 500 );
	std::thread workerThreadFour( SameIDThreadFunc, 4, 1, connectorTwo, static_cast< OutputStamp& >( LineStamp_t<char>::GetInstance() ), 500 );

	std::thread workerThreadFive( SameIDThreadFunc, 5, 0, connectorOne, SystemTimeStamp_t<char>::GetInstance(), 500 );
	std::thread workerThreadSix( SameIDThreadFunc, 6, 1,  connectorTwo, SystemTimeStamp_t<char>::GetInstance(), 500 );
	// wait for all threads to exit naturally
	workerThreadTwo.join();
	workerThreadOne.join();
	workerThreadThree.join();
	workerThreadFour.join();
	workerThreadFive.join();
	workerThreadSix.join();

	streamOne << "Stream One after all channels disconnect..." << endl;
	streamTwo << "Stream Two after all channels disconnect..." << endl;

	cleanupVerify();

	EXPECT_EQ( allOK, true );
	CleanupFiles();
}


TEST( StreamTests, TestConvertingStreamsAllStrings )
{
	bool allOK;

	// squirt the UTF8 strings through converting memory OutputTargets so they go through of all conversion permutations and then compare the final result against the first buffer
	StreamMem< char > strReference;
	StreamMemConverting< wchar_t > streamWide;
	StreamMemConverting< char16_t > strUTF16;
	StreamMemConverting< char32_t > strUTF32;
	StreamMemConverting< char > strDestination;

	// help for the debugger to see the memory buffers and because I need to reset them
 	OutputMem_t< char >& baseRef = strReference.GetOutputTarget();
 	OutputMem_t< wchar_t >& wideMemRef = streamWide.GetOutputTarget();
 	OutputMem_t< char16_t >& sixteenRef = strUTF16.GetOutputTarget();
 	OutputMem_t< char32_t >& thirtytwoRef = strUTF32.GetOutputTarget();
 	OutputMem_t< char >& destRef = strDestination.GetOutputTarget();

	// first send the UTF8 strings into the reference.
	size_t numStrings = sizeof( AllUTF8Strings ) / sizeof( char const * );
	for ( auto * i : AllUTF8Strings )
	{
		strReference << i;
		// we have to explicitly add terminators
		strReference.put( '\0' );
		strReference.flush();
	}

	size_t offset = 0;
	// UTF8 to UTF32
	for( size_t i = 0; i < numStrings; ++i )
	{
		strUTF32 << baseRef.GetBase() + offset;
		offset += strings::GetUTF8StringLengthInBytes( baseRef.GetBase() + offset ) + 1;
		strUTF32.put( U'\0' );
		strUTF32.flush();
	}
	// UTF32 to UTF16
	offset = 0;
	for ( size_t i = 0; i < numStrings; ++i )
	{
		char const * buffPtrInBytes = reinterpret_cast< char const * >( thirtytwoRef.GetBase() ) + offset;
		strUTF16 << reinterpret_cast< char32_t const * >( buffPtrInBytes );
		offset += ( strings::GetUTF32StringLengthInCharacters( buffPtrInBytes ) + 1 ) << 2 ;
		strUTF16.put( u'\0' );
		strUTF16.flush();
	}
	// UTF16 to UTF32
	thirtytwoRef.Reset();
	offset = 0;
	for ( size_t i = 0; i < numStrings; ++i )
	{
		char const * buffPtrInBytes = reinterpret_cast< char const * >( sixteenRef.GetBase() ) + offset;
		strUTF32 << reinterpret_cast< char16_t const * >( buffPtrInBytes );
		offset += strings::GetUTF16StringLengthInBytes( buffPtrInBytes ) + 2;
		strUTF32.put( U'\0' );
		strUTF32.flush();
	}
	// UTF32 to UTF8
	offset = 0;
	for ( size_t i = 0; i < numStrings; ++i )
	{
		char const * buffPtrInBytes = reinterpret_cast< char const * >( thirtytwoRef.GetBase() ) + offset;
		strDestination << reinterpret_cast< char32_t const * >( buffPtrInBytes );
		offset += ( strings::GetUTF32StringLengthInCharacters( buffPtrInBytes ) + 1 ) << 2;
		strDestination.put( '\0' );
		strDestination.flush();
	}
	// UTF8 to UTF16
	offset = 0;
	for ( size_t i = 0; i < numStrings; ++i )
	{
		strUTF16 << baseRef.GetBase() + offset;
		offset += strings::GetUTF8StringLengthInBytes( baseRef.GetBase() + offset ) + 1;
		strUTF16.put( u'\0' );
		strUTF16.flush();
	}
	// UTF16 to UTF8
	offset = 0;
	destRef.Reset();
	for ( size_t i = 0; i < numStrings; ++i )
	{
		char const * buffPtrInBytes = reinterpret_cast< char const * >( sixteenRef.GetBase() ) + offset;
		strDestination << reinterpret_cast< char16_t const * >( buffPtrInBytes );
		offset += strings::GetUTF16StringLengthInBytes( buffPtrInBytes ) + 2;
		strDestination.put( '\0' );
		strDestination.flush();
	}
	allOK = destRef.GetPtr() - destRef.GetBase() == baseRef.GetPtr() - baseRef.GetBase();
	auto sizeReference = baseRef.GetSize();
	auto sizeDestination = destRef.GetSize();
	allOK &= sizeReference == sizeDestination;
	allOK &= 0 == memcmp( destRef.GetBase(), baseRef.GetBase(), sizeReference );
	EXPECT_EQ( allOK, true );
}




