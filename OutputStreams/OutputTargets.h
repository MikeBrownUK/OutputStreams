//////////////////////////////////////////////////////////////////////////
/// Mike Brown, 2022
///
///	Filename: 	OutputTargets.h
///	Created:	21/06/2022
///	Author:		Mike Brown
///	
///	Description: OutputTarget interface classes used by OutputStreams
///
//////////////////////////////////////////////////////////////////////////

#ifndef OutputTarget_DEFINED_21_06_2022
#define OutputTarget_DEFINED_21_06_2022

#if defined ( _MSC_VER )
#include "windows.h"	// for the Windows GUI Console example and the Visual Studio output window example
#elif defined (__linux)
#include "fcntl.h"
#include "unistd.h"
#endif	// #if defined ( _MSC_VER )

#include <fstream>
#include <iostream>
#include <sstream>
#include "assert.h"

namespace mbp
{
	namespace streams
	{
		// Output to a File
		template< typename ELEM_ >
		class OutputFile_t
		{
		public:
			OutputFile_t( char const * const initString_ )
				: m_opened( false )
			{
				std::stringstream filename;
				filename << initString_;
				m_filename = filename.str();
				OpenAndTruncate();
			}

			void OpenAndTruncate()
			{
				std::basic_ofstream< ELEM_, std::char_traits< ELEM_ > > outFile( m_filename.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );
				if ( outFile.good() )
				{
					outFile.seekp( 0, std::ios_base::end );
					if ( outFile.tellp() != 0 )
						return;
					m_opened = true;
				}
			}

			// Output uses kernel calls to write the buffer to a file due to the bemusing way the standard libraries natively (don't) handle wide character file output 
			void Output( ELEM_ const * output_, uint32_t numCharacters_, uint32_t numBytes_ )
			{
				if ( !m_opened )
					OpenAndTruncate();
				if ( m_opened )
				{
#if defined( _MSC_VER )
					HANDLE file = CreateFileA( m_filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL );
					if ( file != INVALID_HANDLE_VALUE )
					{
						if ( INVALID_SET_FILE_POINTER != SetFilePointer( file, 0, 0, 2 ) )
						{
							DWORD bytesWritten;
							WriteFile( file, output_, numBytes_, &bytesWritten, NULL );
						}
						CloseHandle( file );
					}
#elif defined ( __linux )
					int desc = open( m_filename.c_str(), O_WRONLY );
					int numWritten;
					if ( desc > -1 )
					{
						lseek( desc, 0, SEEK_END );
						numWritten = write( desc, output_, numBytes_ );
						if ( numWritten > -1 )
							close( desc );
					}
#endif //#if defined( _MSC_VER )
				}
			}

		private:
			std::string m_filename;
			bool m_opened;				// flags that the file has been opened and truncated successfully
		};
	
		// Output to std::cout or std::wcout (latter requires USE_STD_WCOUT defined)
		template< typename ELEM_ >
		class OutputStdOut_t
		{
		public:
			OutputStdOut_t( char const * const initString_ = nullptr )
			{}
			void Output( ELEM_ const * output_, uint32_t numCharacters_, uint32_t numBytes_ )
			{
				std::cout.write( reinterpret_cast< char const * >( output_ ), numCharacters_ * sizeof( ELEM_ ) );
				std::cout.flush();
			}
		};
	
		template<>
		inline void OutputStdOut_t< wchar_t >::Output( wchar_t const * output_, uint32_t numCharacters_, uint32_t numBytes_ )
		{
#if defined ( USE_STD_WCOUT )
			std::wcout.write( output_, numCharacters_ );
			std::wcout.flush();
#else
			std::cout.write( reinterpret_cast< char const * >( output_ ), numCharacters_ * sizeof( wchar_t ) );
			std::cout.flush();
#endif // #if defined( _MSC_VER )
		}

		// A memory buffer output target. Really just for my test suite so not optimised in any way. Reallocations just attempt to double the current buffer size until we reach a maximum doubling value, beyond which that value is used as an incremental addition.
		auto constexpr kInitialChunkSize = 1024;
		auto constexpr kMaxBeforeAddition = kInitialChunkSize * kInitialChunkSize;
		template< typename ELEM_ >
		class OutputMem_t
		{
		public:
			OutputMem_t( char const * const initString_ )
				: m_currentSize( kInitialChunkSize )
				, m_pBase( nullptr )
				, m_offset( 0 )
			{
				Allocate();
			}
			void Grow( size_t bytesNeeded_ )
			{
				size_t oldSize = m_currentSize;
				size_t newSize = oldSize < kMaxBeforeAddition ? oldSize << 1 : oldSize + kMaxBeforeAddition;
				while ( bytesNeeded_ > newSize - oldSize )
				{
					oldSize = newSize;
					newSize = oldSize < kMaxBeforeAddition ? oldSize << 1 : oldSize + kMaxBeforeAddition;
				}

				ELEM_ * pNewBuff = reinterpret_cast< ELEM_ * >( std::malloc( newSize ) );
				if ( pNewBuff )
				{
					memcpy( pNewBuff, m_pBase, m_offset );
					m_currentSize = newSize;
					std::free( m_pBase );
					m_pBase = pNewBuff;
				}
				else
					assert( false );
			}
			void Reset()
			{
				m_offset = 0;
			}
			void ShrinkAndReset( size_t newSize_ = kInitialChunkSize )
			{
				Release();
				Allocate( newSize_ );
				Reset();
			}
			void Allocate( size_t newSize_ = kInitialChunkSize )
			{
				m_pBase = reinterpret_cast< ELEM_ * >( std::malloc( newSize_ ) );
				m_currentSize = newSize_;
			}
			void Release()
			{
				if ( m_pBase )
					free( m_pBase );
				m_pBase = nullptr;
				m_currentSize = m_offset = 0;
			}
			~OutputMem_t()
			{
				Release();
			}
			void Output( ELEM_ const * output_, uint32_t numCharacters_, uint32_t numBytes_ )
			{
				if( m_offset + numBytes_ >= m_currentSize )
					Grow( numBytes_ - ( m_currentSize - m_offset ) );
				memcpy( reinterpret_cast< char * >( m_pBase ) + m_offset, output_, numBytes_ );
				m_offset += numBytes_;
			}
			ELEM_ const * GetBase() const { return m_pBase; }
			ELEM_ const * GetPtr() const { return reinterpret_cast< char const * >( m_pBase ) + m_offset; }
			size_t GetSize() const { return m_currentSize; }
			size_t GetRemaining() const { return m_currentSize - m_offset; }
		private:
			size_t m_currentSize;
			size_t m_offset;
			ELEM_ * m_pBase;
		};

		/////////////////////////////////////////
		/// Windows only OutputTarget examples
		/////////////////////////////////////////

#if defined(_MSC_VER)
		// output to the Windows debugger (usually Visual Studio's output window)
		template< typename ELEM_ >
		class OutputDevStudio_t
		{
		public:
			OutputDevStudio_t( char const * const initString_ = nullptr )
			{}
			void Output( ELEM_ const * output_, uint32_t numCharacters_, uint32_t numBytes_ )
			{
				OutputDebugStringW( output_ );
			}
		};

		template<>
		inline void OutputDevStudio_t< char >::Output( char const * output_, uint32_t numCharacters_, uint32_t numBytes_ )
		{
			OutputDebugStringA( output_ );
		}

		// output to console window - Windows
		template< typename ELEM_ >
		class OutputConsole_t
		{
		public:
			OutputConsole_t( char const * const initString_ = nullptr )
				: m_bDetach( true )
			{
				if ( 0 == AllocConsole() )
					m_bDetach = false;

				m_hOutput = GetStdHandle( STD_OUTPUT_HANDLE );

				CONSOLE_SCREEN_BUFFER_INFO ci;

				GetConsoleScreenBufferInfo( m_hOutput, &ci );

				COORD c;

				c.Y = ci.dwMaximumWindowSize.Y;
				c.X = ci.dwMaximumWindowSize.X;

				SetConsoleScreenBufferSize( m_hOutput, c );

				HWND win = GetConsoleWindow();
				ShowWindow( win, SW_SHOWNORMAL );

			}

			~OutputConsole_t()
			{
				if( m_bDetach )
					FreeConsole();
			}

			inline void Output( typename ELEM_ const * output_, uint32_t numCharacters_, uint32_t numBytes_ )
			{
				DWORD dTemp;
				WriteConsoleW( m_hOutput, output_, numCharacters_, &dTemp, NULL );
			}

		private:
			HWND GetConsoleWindow()
			{
				wchar_t szWindowTitle[ 250 ];

				GetConsoleTitleW( szWindowTitle, 250 );

				HWND hFound;
				do
				{
					Sleep( 0 );
					hFound = FindWindowW( NULL, szWindowTitle );
				}
				while ( !hFound );
				return hFound;
			}
			HANDLE m_hOutput;
			bool m_bDetach;
		};

		template<>
		inline void OutputConsole_t< char >::Output( char const * output_, uint32_t numCharacters_, uint32_t numBytes_ )
		{
			DWORD dTemp;
			WriteConsoleA( m_hOutput, output_, numCharacters_, &dTemp, NULL );
		}
#endif //#if defined(_MSC_VER)
	}
}
#endif // #ifndef OutputTarget_DEFINED_21_06_2022
