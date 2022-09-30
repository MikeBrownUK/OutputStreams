//////////////////////////////////////////////////////////////////////////
/// Mike Brown, 2014-2022
///
///	Filename: 	OutputStreams.h
///	Created:	31/12/2014
///	Author:		Mike Brown
///	
///	Description: OutputStreams for Windows and Linux for runtime logging and diagnostics with optional line stamping
///				 
/// Code generated by OutputStreams can be removed from execution paths in an optimised build by defining STREAM_OUTPUT_STRIP
///
//////////////////////////////////////////////////////////////////////////

#ifndef OutputStreams_DEFINED_31_12_2014
#define OutputStreams_DEFINED_31_12_2014

#include <mutex>
#include <iostream>
#include <sstream>
#include <atomic>
#include <iomanip>
#if defined( __linux )
#include <cstring>
#endif
#include <vector>

#include "OutputTargets.h"
#include "OutputStamp.h"
#include "Utilities/Strings.h"

namespace mbp
{
	namespace streams
	{
		//////////////////////////////////////////////////////////////////////////
		/// Settings used by OutputStreams 
		//////////////////////////////////////////////////////////////////////////

		using SettingsType = uint8_t;
		// message priority is the inverse of what might be expected: lowest value is highest priority
		// 0 = highest priority ... ~0 = lowest priority
		constexpr SettingsType kPriorityMax = 0;
		constexpr SettingsType kPriorityDefault = 1;
		constexpr SettingsType kDefaultFilter = static_cast< SettingsType >( ~0 );	// Messages with priority > current filter value are not output

		struct StreamSettings
		{
			StreamSettings( SettingsType enable_ = 1, SettingsType initPriority_ = kPriorityDefault, SettingsType initialFilter_ = kDefaultFilter )
				: enabled( enable_ ), currentPriority( initPriority_ ), defaultPriority( initPriority_ ), filter( initialFilter_ )
			{}

			std::atomic< SettingsType > enabled;
			std::atomic< SettingsType > currentPriority;
			std::atomic< SettingsType > defaultPriority;
			std::atomic< SettingsType > filter;

			void Enable( SettingsType enable_ ) { enabled.store( enable_, std::memory_order_relaxed ); }
			SettingsType GetEnable() { return enabled.load( std::memory_order_relaxed ); }
			void SetPriority( SettingsType newPriority_ ) { currentPriority.store( newPriority_, std::memory_order_relaxed ); }
			SettingsType GetPriority() { return currentPriority.load( std::memory_order_relaxed ); }
			void SetDefaultPriority( SettingsType newDefault_ ) {
				defaultPriority.store( newDefault_, std::memory_order_relaxed ); currentPriority.store( newDefault_, std::memory_order_relaxed );
			}
			SettingsType GetDefaultPriority() { return defaultPriority.load( std::memory_order_relaxed ); }
			void SetFilter( SettingsType newFilter_ ) { filter.store( newFilter_, std::memory_order_relaxed ); }
			SettingsType GetFilter() { return filter.load( std::memory_order_relaxed ); }

			void ResetDefault() { currentPriority.store( defaultPriority, std::memory_order_relaxed ); }
			bool CanBeOutput() {
				bool ok = enabled.load( std::memory_order_relaxed ) && ( currentPriority.load( std::memory_order_relaxed ) <= filter.load( std::memory_order_relaxed ) );
				return ok;
			}
		};

		// The default StreamSettings instance
		extern StreamSettings & GetDefaultChannelSettings();

		// flags cleared by the OutputStream destructor
		extern void * g_allSharedStreams[];
		extern uint8_t g_streamInitFlags[];
		// mutex protecting the above
		extern std::mutex g_streamsMutex;
		// hash and probe function
		extern size_t GetIndexFromPointer( void * ptr_ );

		//////////////////////////////////////////////////////////////////////////
		/// BasicStream
		//////////////////////////////////////////////////////////////////////////

		template < typename ELEM_ >
		class BasicStream_t : public std::basic_ostream< ELEM_, std::char_traits< ELEM_ > >
		{
		public:
			using traits = std::char_traits< ELEM_ >;
			BasicStream_t( std::basic_stringbuf< ELEM_, traits, std::allocator< ELEM_ > > * buffer_, StreamSettings * initSettings_, OutputStamp & stamp_ )
				: std::basic_ostream< ELEM_, traits >( buffer_ )
				, m_stamp( stamp_ )
				, m_isChannelTarget( false )
			{
				m_settings.Enable( initSettings_->GetEnable() );
				m_settings.SetPriority( initSettings_->GetPriority() );
				m_settings.SetDefaultPriority( initSettings_->GetDefaultPriority() );
				m_settings.SetFilter( initSettings_->GetFilter() );
			}
			virtual ~BasicStream_t() {}

			virtual void Enable( SettingsType enable_ ) { m_settings.Enable( enable_ ); }
			virtual void SetPriority( SettingsType newPriority_ ) { m_settings.SetPriority( newPriority_ ); }
			virtual void SetDefaultPriority( SettingsType newDefault_ ) { m_settings.SetDefaultPriority( newDefault_ ); }
			virtual void SetFilter( SettingsType newCap_ ) { m_settings.SetFilter( newCap_ ); }
			virtual SettingsType GetEnable() { return m_settings.GetEnable(); }
			virtual SettingsType GetPriority() { return m_settings.GetPriority(); }
			virtual SettingsType GetDefaultPriority() { return m_settings.GetDefaultPriority(); }
			virtual SettingsType GetFilter() { return m_settings.GetFilter(); }

			// access functions when the stream is a shared target
			void Lock() { m_lock.lock(); }
			void Unlock() { m_lock.unlock(); }
			bool TryLock() { return m_lock.try_lock(); }
			void SetIsChannelTarget( bool isShared_ ) { m_isChannelTarget.store( isShared_, std::memory_order_release ); }
			bool GetIsChannelTarget() { return m_isChannelTarget.load( std::memory_order_acquire ); }

			OutputStamp& GetOutputStamp() { return m_stamp; }
			
			StreamSettings m_settings;
		protected:
			std::mutex m_lock;
			OutputStamp & m_stamp;
			std::atomic< bool > m_isChannelTarget;
			BasicStream_t() = delete;
			BasicStream_t( BasicStream_t const & other_ ) = delete;
			BasicStream_t operator=( BasicStream_t const & other_ ) = delete;
		};

		// default BasicStream_t derivative performs integral narrowing / widening on non-native character pointers
		template< typename ELEM_ >
		class Stream_t : public BasicStream_t< ELEM_ >
		{
		public:
			using base = BasicStream_t< ELEM_ >;
			using type = ELEM_;
			using traits = std::char_traits< ELEM_ >;
			Stream_t( std::basic_stringbuf< ELEM_, traits, std::allocator< ELEM_ > > * buffer_, StreamSettings * initSettings_, OutputStamp & stamp_ )
				: BasicStream_t< ELEM_ >( buffer_, initSettings_, stamp_ )
			{}
			virtual ~Stream_t() {}
		};

		// specialisation whose helpers convert non-native strings to the UTF format most appropriate to the stream width
		template< typename ELEM_ >
		class ConvertingStream_t : public BasicStream_t< ELEM_  >
		{
		public:
			using base = BasicStream_t< ELEM_ >;
			using type = ELEM_;
			using traits = std::char_traits< ELEM_ >;
			ConvertingStream_t( std::basic_stringbuf< ELEM_, traits, std::allocator< ELEM_ > > * buffer_, StreamSettings * initSettings_, OutputStamp & stamp_ )
				: BasicStream_t< ELEM_ >( buffer_, initSettings_, stamp_ )
			{}
			virtual ~ConvertingStream_t() {}
		};

		//////////////////////////////////////////////////////////////////////////
		/// BasicBuffer
		//////////////////////////////////////////////////////////////////////////

		template < class ELEM_ >
		class BasicBuffer_t : public std::basic_stringbuf< ELEM_, std::char_traits< ELEM_ >, std::allocator< ELEM_ > >
		{
			using traits = std::char_traits< ELEM_ >;
			using base = std::basic_stringbuf< ELEM_, traits, std::allocator< ELEM_ > >;
		public:
			BasicBuffer_t( BasicStream_t< ELEM_ > & stream_ )
			{
				ReserveStamp( stream_ );
			}
			void ReserveStamp( BasicStream_t< ELEM_ > & stream_ )
			{
				auto len = stream_.GetOutputStamp().GetMaxLength();
				for ( auto i = 0; i < len; ++i )
				{
					base::sputc( static_cast< ELEM_ >( 'B' ) );
				}
			}
			virtual ~BasicBuffer_t() {}
			void SetOriginalBufferStart()
			{
				ptrdiff_t pPtr = reinterpret_cast< ptrdiff_t >( base::pptr() );
				ptrdiff_t pBase = reinterpret_cast< ptrdiff_t >( base::pbase() );
				ptrdiff_t diff = ( pPtr - pBase ) / sizeof( ELEM_ );
				base::pbump( -static_cast< int >( diff ) );
			}
		private:
			BasicBuffer_t() = delete;
			BasicBuffer_t( BasicBuffer_t const & other_ ) = delete;
			BasicBuffer_t operator=( BasicBuffer_t const & other_ ) = delete;
		};

		//////////////////////////////////////////////////////////////////////////
		/// OutputBuffer - flushes to an OutputTarget<> template class
		//////////////////////////////////////////////////////////////////////////

		template < typename ELEM_, template< typename > typename TARGET_  >
		class OutputBuffer_t : public BasicBuffer_t< ELEM_ >
		{
		public:
			using base = BasicBuffer_t< ELEM_ >;
			OutputBuffer_t( BasicStream_t< ELEM_ > & stream_, char const * const fileName_ = nullptr )
				: BasicBuffer_t< ELEM_ >( stream_ )
				, m_stream( stream_ )
				, m_outputTarget( fileName_ )
			{}
			virtual ~OutputBuffer_t() = default;
			TARGET_< ELEM_ > & GetOutputTarget() { return m_outputTarget; }
		protected:
			virtual int sync() override
			{
				int maxLength = 0;
				uint32_t numCharacters = static_cast< uint32_t >( base::pptr() - base::pbase() );
				if ( m_stream.m_settings.CanBeOutput() )
				{
					int stampLength = 0;
					int offset = 0;
					auto isTarget = m_stream.GetIsChannelTarget();
					if ( !isTarget )
					{
						maxLength = m_stream.GetOutputStamp().GetMaxLength();
						stampLength = m_stream.GetOutputStamp().GetLength();
						offset = maxLength - stampLength;
						m_stream.GetOutputStamp().WriteStamp( base::pbase() + offset );
					}
						
					// get the number of bytes to pass to the OutputTarget - useful for direct writes using system functions
					uint32_t numBytes = ( numCharacters - offset ) * sizeof( ELEM_ );
					// add a terminating zero character - OutputDebugString requires zero terminated strings, as might other OutputTarget implementations
					base::sputc( 0 );
					m_outputTarget.Output( base::pbase() + offset, numCharacters++ - offset, numBytes );
				}
				base::pbump( -static_cast< int >( numCharacters - maxLength ) );
				// we reset priority level to default priority following each flush
				m_stream.m_settings.SetPriority( m_stream.m_settings.GetDefaultPriority() );
				return 0;
			}
		private:
			OutputBuffer_t() = delete;
			OutputBuffer_t( OutputBuffer_t const & rhs_ ) = delete;
			OutputBuffer_t & operator = ( OutputBuffer_t const & rhs_ ) = delete;
			BasicStream_t< ELEM_ > & m_stream;
			TARGET_< ELEM_ > m_outputTarget;
		};

		//////////////////////////////////////////////////////////////////////////
		/// OutputStream class. Needs base class and OutputTarget via template params
		/// Base class will depend on whether UTF conversions are required
		//////////////////////////////////////////////////////////////////////////

		template< typename ELEM_, template< typename > typename TARGET_, template< typename > typename STREAMBASE_ >
		class OutputStream_t : public STREAMBASE_< ELEM_ >
		{
		public:
			OutputStream_t( char const * const initString_ = nullptr, OutputStamp & stamp_ = OutputStamp::GetDummyStamp(), StreamSettings * initialSettings_ = &GetDefaultChannelSettings() )
				: STREAMBASE_< ELEM_ >( &m_buffer, initialSettings_, stamp_ )
				, m_buffer( *this, initString_ )
			{}
			virtual ~OutputStream_t()
			{
				if ( STREAMBASE_< ELEM_ >::GetIsChannelTarget() )
				{
					// ensure OutputChannels know we have been destroyed
					size_t index = GetIndexFromPointer( this );
					g_streamsMutex.lock();
					g_streamInitFlags[ index ] = 0;
					g_allSharedStreams[ index ] = nullptr;
					g_streamsMutex.unlock();
				}
			}
			TARGET_< ELEM_ > & GetOutputTarget() { return m_buffer.GetOutputTarget(); }
		protected:
			OutputBuffer_t< ELEM_, TARGET_ > m_buffer;
			OutputStream_t( OutputStream_t const & other_ ) = delete;
			OutputStream_t operator=( OutputStream_t const & other_ ) = delete;
		};


		//////////////////////////////////////////////////////////////////////////
		/// A specialisation of OutputStream which captures another stream's buffer (e.g. std::out) and diverts it to an OutputTarget
		//////////////////////////////////////////////////////////////////////////

		template< typename ELEM_, template< typename > typename STREAMBASE_, template< typename > typename TARGET_ >
		class OutputStreamCapture_t : public OutputStream_t< ELEM_, STREAMBASE_, TARGET_ >
		{
		public:
			OutputStreamCapture_t( StreamSettings * initialSettings_, char const * const fileName_ = nullptr, std::basic_ostream< ELEM_ > * stream_ = nullptr )
				: OutputStream_t< ELEM_, TARGET_, STREAMBASE_>( initialSettings_, fileName_ )
				, m_prevStream( stream_ )
			{
				if ( m_prevStream )
					m_prevStreamBuf = m_prevStream->rdbuf( this->rdbuf() );
			}

			virtual ~OutputStreamCapture_t()
			{
				if ( m_prevStreamBuf )
					m_prevStream->rdbuf( m_prevStreamBuf );
			}

		private:
			OutputStream_t< ELEM_, STREAMBASE_, TARGET_ > * m_prevStream;
			std::basic_streambuf< ELEM_, std::char_traits< ELEM_  > > * m_prevStreamBuf;

			OutputStreamCapture_t() = delete;
			OutputStreamCapture_t( OutputStreamCapture_t const & other_ ) = delete;
			OutputStreamCapture_t operator=( OutputStreamCapture_t const & other_ ) = delete;
		};

		//////////////////////////////////////////////////////////////////////////
		/// A custom definition of endl because std::endl on Linux throws an exception with char16_t and char32_t streams
		//////////////////////////////////////////////////////////////////////////

		template< typename ELEM_ >
		std::basic_ostream< ELEM_, std::char_traits< ELEM_ > > & endl( std::basic_ostream< ELEM_, std::char_traits< ELEM_ > > & stream_ )
		{
			stream_.rdbuf()->sputc( static_cast< ELEM_ >( '\n' ) );
			stream_.flush();
			return stream_;
		}

		// the stream function to forward to the actual manipulator
		template< typename ELEM_ >
		inline BasicStream_t< ELEM_ > & operator <<( BasicStream_t< ELEM_ > & stream_, BasicStream_t< ELEM_ > & ( *fnc_ )( BasicStream_t< ELEM_ > & ) )
		{
			return fnc_( stream_ );
		}

		// and a customisation of the setw manipulator and caller to ensure code is stripped properly when using NullStream instead of OutputStream or OutputChannel
#define stdManipOne(manip,fnc)	struct manip { explicit manip( int val_ ) : payload_( val_ ) {} template< typename T_ > T_ & operator()( T_ && strm_ )	{ strm_.fnc( payload_ ); return strm_; } int payload_; };

		stdManipOne( setw, width )

		template< typename ELEM_ >
		std::basic_ostream< ELEM_ > & operator << ( std::basic_ostream< ELEM_ > & stream_, setw && obj_ )
		{
			obj_( stream_ );
			return stream_;
		}

		//////////////////////////////////////////////////////////////////////////
		/// OutputStream and OutputChannel manipulator functors
		//////////////////////////////////////////////////////////////////////////

		struct Enable
		{
			explicit Enable( int val_ )
				: payload_( val_ )
			{}
			template< typename T_ >
			T_ & operator()( T_ && strm_ )
			{
				strm_.Enable( payload_ );
				return strm_;
			}
			int payload_;
		};

		struct Priority
		{
			explicit Priority( int val_ )
				: payload_( val_ )
			{}
			template< typename T_ >
			T_ & operator()( T_ && strm_ )
			{
				strm_.SetPriority( payload_ );
				return strm_;
			}
			int payload_;
		};

		struct DefaultPriority
		{
			explicit DefaultPriority( int val_ )
				: payload_( val_ )
			{}
			template< typename T_ >
			T_ & operator()( T_ && strm_ )
			{
				strm_.SetDefaultPriority( payload_ );
				strm_.SetPriority( payload_ );
				return strm_;
			}
			int payload_;
		};

		struct Filter
		{
			explicit Filter( int val_ )
				: payload_( val_ )
			{}
			template< typename T_ >
			T_ & operator()( T_ && strm_ )
			{
				strm_.SetFilter( payload_ );
				return strm_;
			}
			int payload_;
		};

		// and their stream forwarders

		template< typename ELEM_ >
		BasicStream_t< ELEM_ > & operator << ( BasicStream_t< ELEM_ > & stream_, Enable && obj_ )
		{
			return obj_( stream_ );
		}
		template< typename ELEM_ >
		BasicStream_t< ELEM_ > & operator << ( BasicStream_t< ELEM_ > & stream_, Priority && obj_ )
		{
			return obj_( stream_ );
		}
		template< typename ELEM_ >
		BasicStream_t< ELEM_ > & operator << ( BasicStream_t< ELEM_ > & stream_, DefaultPriority && obj_ )
		{
			return obj_( stream_ );
		}
		template< typename ELEM_ >
		BasicStream_t< ELEM_ > & operator << ( BasicStream_t< ELEM_ > & stream_, Filter && obj_ )
		{
			return obj_( stream_ );
		}

		//////////////////////////////////////////////////////////////////////////
		/// Stream and Channel string pointer handler for non-converting streams performs integral narrowing/widening
		//////////////////////////////////////////////////////////////////////////

		template< typename ELEM_, typename OTHER_ >
		Stream_t< ELEM_ > & TextCatcher( Stream_t< ELEM_ > & stream_, OTHER_ const * pSource_ )
		{
			OTHER_ character;
			while ( character = *pSource_++ )
				stream_.rdbuf()->sputc( static_cast< ELEM_ >( character ) );
			return stream_;
		}

		//////////////////////////////////////////////////////////////////////////
		/// String conversion pointer functions for ConvertStream_t< ELEM_ >
		/// We have to make assumptions of course - see function descriptions. Also note that code assumes little endian data and target at present.
		//////////////////////////////////////////////////////////////////////////

		// catch all char32_t pointers, assume UTF32 and and transform into UTF8
		ConvertingStream_t< char > & ConvertText( ConvertingStream_t< char > & stream_, char32_t const * pSource_ );
		// catch all char32_t pointers and transform into UTF16
		ConvertingStream_t< char16_t > & ConvertText( ConvertingStream_t< char16_t > & stream_, char32_t const * pSource_ );
		// catch all char16_t pointers and transforms into UTF8
		ConvertingStream_t< char > & ConvertText( ConvertingStream_t< char> & stream_, char16_t const * pSource_ );
		// catch all char16_t pointers and transforms to UTF32
		ConvertingStream_t< char32_t > & ConvertText( ConvertingStream_t< char32_t > & stream_, char16_t const * pSource_ );
		// catch all char pointers and transforms to UTF32
		ConvertingStream_t< char32_t > & ConvertText( ConvertingStream_t< char32_t > & stream_, char const * pSource_ );
		// catch all char pointers and transforms to UTF16
		ConvertingStream_t< char16_t > & ConvertText( ConvertingStream_t< char16_t > & stream_, char const * pSource_ );

		//////////////////////////////////////////////////////////////////////////
		/// platform dependent further conversion functions just for wchar_t
		//////////////////////////////////////////////////////////////////////////

#if defined( __linux )
		// catch all char pointers, assume UTF8 and transform to UTF32 wchar_t for linux
		ConvertingStream_t< wchar_t > & ConvertText( ConvertingStream_t< wchar_t > & stream_, char const * pSource_ );
		// catch all char16_t pointers, assume UTF16 and transform to UTF32
		ConvertingStream_t< wchar_t > & ConvertText( ConvertingStream_t< wchar_t > & stream_, char16_t const * pSource_ );
		// catch all wchar_t pointers, assume UTF32 and transform to UTF16
		ConvertingStream_t< char16_t > & ConvertText( ConvertingStream_t< char16_t > & stream_, wchar_t const * pSource_ );
		// catch all wchar_t pointers, assume UTF32 and transform to UTF8
		ConvertingStream_t< char > & ConvertText( ConvertingStream_t< char > & stream_, wchar_t const * pSource_ );
#elif defined ( _MSC_VER )
		// catch char pointers, assume UTF8 and transform to UTF16 for Microsoft and other 16-bit wchar_t platforms
		ConvertingStream_t< wchar_t > & ConvertText( ConvertingStream_t< wchar_t > & stream_, char const * pSource_ );
		// catch all char32_t pointers, assume UTF32 and transform to UTF16
		ConvertingStream_t< wchar_t > & ConvertText( ConvertingStream_t< wchar_t > & stream_, char32_t const * pSource_ );
		// catch all wchar_t pointers, assume UTF16 and transform to UTF8
		ConvertingStream_t< char > & ConvertText( ConvertingStream_t< char > & stream_, wchar_t const * pSource_ );
		// catch all wchar_t pointers, assume UTF16 and transform to UTF32
		ConvertingStream_t< char32_t > & ConvertText( ConvertingStream_t< char32_t > & stream_, wchar_t const * pSource_ );
#endif //#if defined( __linux )...

		//////////////////////////////////////////////////////////////////////////
		/// Operator << string pointer specialisations for Stream_t< ELEM_ >
		/// These forward to the single integral string conversion function so we don't have to display a meaningless pointer value, or perform a direct write where the types are the same
		//////////////////////////////////////////////////////////////////////////

		template< typename ELEM_ >
		Stream_t< ELEM_ > & operator << ( Stream_t< ELEM_ > & stream_, char const * pSource_ )
		{
			return TextCatcher< ELEM_, char >( stream_, pSource_ );
		}

		template< typename ELEM_ >
		Stream_t< ELEM_ > & operator << ( Stream_t< ELEM_ > & stream_, wchar_t const * pSource_ )
		{
			return TextCatcher< ELEM_, wchar_t >( stream_, pSource_ );
		}

		template< typename ELEM_ >
		Stream_t< ELEM_ > & operator << ( Stream_t< ELEM_ > & stream_, char16_t const * pSource_ )
		{
			return TextCatcher< ELEM_, char16_t >( stream_, pSource_ );
		}

		template< typename ELEM_ >
		Stream_t< ELEM_ > & operator << ( Stream_t< ELEM_ > & stream_, char32_t const * pSource_ )
		{
			return TextCatcher< ELEM_, char32_t >( stream_, pSource_ );
		}

		inline BasicStream_t< char > & operator << ( BasicStream_t< char > & stream_, char const * pSource_ )
		{
			stream_.write( pSource_, strlen( pSource_ ) );
			return stream_;
		}

		inline BasicStream_t< wchar_t > & operator << ( BasicStream_t< wchar_t > & stream_, wchar_t const * pSource_ )
		{
			stream_.write( pSource_, wcslen( pSource_ ) );
			return stream_;
		}

		inline BasicStream_t< char16_t > & operator << ( BasicStream_t< char16_t > & stream_, char16_t const * pSource_ )
		{
			auto numChars = strings::GetUTF16StringLengthInCharacters( pSource_ );
			stream_.rdbuf()->sputn( pSource_, numChars );
			// ...because the below doesn't work as expected on Linux with a char16_t stream
			//stream_.write( pSource_, numChars );
			return stream_;
		}

		inline BasicStream_t< char32_t > & operator << ( BasicStream_t< char32_t > & stream_, char32_t const * pSource_ )
		{
			stream_.write( pSource_, strings::GetUTF32StringLengthInCharacters( pSource_ ) );
			return stream_;
		}

		//////////////////////////////////////////////////////////////////////////
		/// Operator << string pointer specialisations for ConvertStream_t< ELEM_ >
		/// These forward to the appropriate conversion function and again perform a direct write where the types are the same
		//////////////////////////////////////////////////////////////////////////

		template< typename ELEM_ >
		ConvertingStream_t< ELEM_ > & operator << ( ConvertingStream_t< ELEM_ > & stream_, char const * pSource_ )
		{
			return ConvertText( stream_, pSource_ );
		}

		template< typename ELEM_ >
		ConvertingStream_t< ELEM_ > & operator << ( ConvertingStream_t< ELEM_ > & stream_, wchar_t const * pSource_ )
		{
			return ConvertText( stream_, pSource_ );
		}

		template< typename ELEM_ >
		ConvertingStream_t< ELEM_ > & operator << ( ConvertingStream_t< ELEM_ > & stream_, char16_t const * pSource_ )
		{
			return ConvertText( stream_, pSource_ );
		}

		template< typename ELEM_ >
		ConvertingStream_t< ELEM_ > & operator << ( ConvertingStream_t< ELEM_ > & stream_, char32_t const * pSource_ )
		{
			return ConvertText( stream_, pSource_ );
		}

		inline ConvertingStream_t< char > & operator << ( ConvertingStream_t< char > & stream_, char const * pSource_ )
		{
			stream_.write( pSource_, strlen( pSource_ ) );
			return stream_;
		}

		inline ConvertingStream_t< wchar_t > & operator << ( ConvertingStream_t< wchar_t > & stream_, wchar_t const * pSource_ )
		{
			stream_.write( pSource_, wcslen( pSource_ ) );
			return stream_;
		}

		inline ConvertingStream_t< char16_t > & operator << ( ConvertingStream_t< char16_t > & stream_, char16_t const * pSource_ )
		{
			auto numChars = strings::GetUTF16StringLengthInCharacters( pSource_ );
			stream_.rdbuf()->sputn( pSource_, numChars );
			return stream_;
		}

		inline ConvertingStream_t< char32_t > & operator << ( ConvertingStream_t< char32_t > & stream_, char32_t const * pSource_ )
		{
			stream_.write( pSource_, strings::GetUTF32StringLengthInCharacters( pSource_ ) );
			return stream_;
		}

#if defined( _MSC_VER )
		inline ConvertingStream_t< wchar_t > & operator << ( ConvertingStream_t< wchar_t > & stream_, char16_t const * pSource_ )
		{
			stream_.write( reinterpret_cast< wchar_t const * >( pSource_ ), wcslen( reinterpret_cast< wchar_t const * >( pSource_ ) ) );
			return stream_;
		}
		inline ConvertingStream_t< char16_t > & operator << ( ConvertingStream_t< char16_t > & stream_, wchar_t const * pSource_ )
		{
			stream_.write( reinterpret_cast< char16_t const * >( pSource_ ), wcslen( pSource_ ) );
			return stream_;
		}
#elif defined( __linux )
		inline ConvertingStream_t< wchar_t > & operator << ( ConvertingStream_t< wchar_t > & stream_, char32_t const * pSource_ )
		{
			stream_.write( reinterpret_cast< wchar_t const * >( pSource_ ), wcslen( reinterpret_cast< wchar_t const * >( pSource_ ) ) );
			return stream_;
		}
		inline ConvertingStream_t< char32_t > & operator << ( ConvertingStream_t< char32_t > & stream_, wchar_t const * pSource_ )
		{
			stream_.write( reinterpret_cast< char32_t const * >( pSource_ ), wcslen( pSource_ ) );
			return stream_;
		}
#endif

		//////////////////////////////////////////////////////////////////////////
		// NullStream declaration
		//////////////////////////////////////////////////////////////////////////

		template< typename T_ >
		class NullStream_t
		{
		public:
			// OutputStream and Channel member function proxies
			template< typename ... Args_ > inline NullStream_t< T_ >( Args_... args_ ) {}
			inline void Enable( SettingsType dummy_ ) {}
			inline void SetPriority( SettingsType dummy_ ) {}
			inline void SetDefaultPriority( SettingsType dummy_ ) {}
			inline void SetCap( SettingsType dummy_ ) {}
			inline SettingsType GetEnable() { return 0;	}
			inline SettingsType GetPriority() { return 0; }
			inline SettingsType GetFilter() { return 0; }
			inline SettingsType GetDefaultPriority() { return 0; }
			// for common ios_base functions
			template< typename U_ >
			inline void imbue( const U_& dummy_ ) {}
			std::locale getloc() { return std::locale::classic(); }
			inline NullStream_t & flush() { return *this; }
		
		};

		template< typename T_, typename U_ >
		inline NullStream_t< T_ > & operator << ( NullStream_t< T_ >& stream_, U_ const & ) { return stream_; }

		template< typename T_, typename U_ >
 		inline void operator << ( NullStream_t< T_ > & stream_, std::basic_ostream< U_, std::char_traits< U_ > > & ( * )( std::basic_ostream< U_, std::char_traits< U_ > > &) ) {}

		template< typename T_ >
		inline NullStream_t< T_ > & endl( NullStream_t< T_ > & stream_ )
		{
			return stream_;
		}

		template< typename T_ >
		inline NullStream_t< T_ > & operator <<( NullStream_t< T_ > & stream_, NullStream_t< T_ > & ( *fnc_ )( NullStream_t< T_ > & ) )
		{
			return stream_;
		}

	} // namespace streams
} // namespace mbp

#endif // #ifndef OutputStreams_DEFINED_31_12_2014


