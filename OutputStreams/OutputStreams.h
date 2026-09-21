//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/// ©Mike Brown, 2014-2026
/// https://www.mikebrown.co.uk
///
///	Filename: 	OutputStreams.h
///	Created:	28/08/2026
///	Author:		Mike Brown
///	
///	Description: OutputStreams 2.0 for runtime logging & diagnostics with optional line stamping
///				 also very useful for general stream to stream applications, especially with its inbuilt char-type conversion options.
///				 
/// #define OUTPUT_STREAM_STRIP will replace all OutputStream library objects with proxy empty classes
///			(giving compile time disable with compiler code-strip and literal removal in optimal conditions)
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
#include <streambuf>

#include "OutputTargets.h"
#include "OutputStamp.h"
#include "Utilities/Strings.h"

namespace mbp
{
	namespace streams
	{
		// for imbuing streams with test locales
#if defined( _MSC_VER )
		auto constexpr kLocaleGB = "en-GB";
#elif defined( __linux )
		auto constexpr kLocaleGB = "en_GB.utf8";
#endif
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
			SettingsType GetPriority()  { return currentPriority.load( std::memory_order_relaxed ); }
			void SetDefaultPriority( SettingsType newDefault_ ) {
				defaultPriority.store( newDefault_, std::memory_order_relaxed ); currentPriority.store( newDefault_, std::memory_order_relaxed );
			}
			SettingsType GetDefaultPriority()  { return defaultPriority.load( std::memory_order_relaxed ); }
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
		extern void ** GetSharedStreamArray();
		extern uint8_t * GetStreamInitFlags();
		// mutex protecting the above
		extern std::mutex& GetMasterChannelMutex();
		// hash and probe function
		extern size_t GetIndexFromPointer( void * ptr_ );

		//////////////////////////////////////////////////////////////////////////
		// NullStream declaration
		//////////////////////////////////////////////////////////////////////////

		template< typename T_  >
		class NullStream_t
		{
		public:	
			NullStream_t() = default;

			inline void Enable( SettingsType dummy_ ) {}
			inline void SetPriority( SettingsType dummy_ ) {}
			inline void SetDefaultPriority( SettingsType dummy_ ) {}
			inline void SetFilter( SettingsType dummy_ ) {}
			inline SettingsType GetEnable() { return 0; }
			inline SettingsType GetPriority() { return 0; }
			inline SettingsType GetFilter() { return 0; }
			inline SettingsType GetDefaultPriority() { return 0; }
			// for common ios_base functions
			template< typename U_ >
			inline void imbue( const U_& dummy_ ) {}
			std::locale getloc() { return std::locale::classic(); }
			inline NullStream_t& flush() { return *this; }

			static NullStream_t ms_inst;
		};

		//////////////////////////////////////////////////////////////////////////
		/// NullStream forwarders
		//////////////////////////////////////////////////////////////////////////

		template< typename T_, typename U_ >
		NullStream_t< T_ >& operator <<( NullStream_t< T_ >& stream_, U_ const& other )
		{
			return stream_;
		}
		template< typename T_ >
		NullStream_t< T_ > NullStream_t< T_ >::ms_inst;

		template< typename T_ >
		NullStream_t< T_ >& operator<<( NullStream_t< T_ >&, NullStream_t< T_ >& ( *fnc )( NullStream_t< T_ >& ) )
		{
			return NullStream_t< T_ >::ms_inst;
		}

		template< typename T_ >
		NullStream_t< T_ >& operator<<( NullStream_t< T_ >&, std::basic_ostream< T_, std::char_traits< T_ > >& ( *fnc )( std::basic_ostream< T_, std::char_traits< T_ > >& ) )
		{
			return NullStream_t< T_ >::ms_inst;
		}
	
#if !defined( OUTPUT_STREAM_STRIP )

		//////////////////////////////////////////////////////////////////////////
		/// BasicStream
		//////////////////////////////////////////////////////////////////////////

		// forward dec
		template < class ELEM_ >
		class BasicBuffer_t;

		template < typename ELEM_ >
		class BasicStream_t : public std::basic_ostream< ELEM_, std::char_traits< ELEM_ > >
		{
		public:
			using elem = ELEM_;
			using base = std::basic_ostream< ELEM_, std::char_traits< ELEM_ > >;
			BasicStream_t( BasicBuffer_t< elem > * buffer_ )
				: std::basic_ostream< elem, std::char_traits< elem > >( buffer_ )
			{
			}
			virtual ~BasicStream_t() {}

			// buffer forwarders
			virtual void Enable( SettingsType enable_ ) { static_cast< BasicBuffer_t< elem > * >( base::rdbuf() )->Enable( enable_ ); }
			virtual void SetPriority( SettingsType newPriority_ ) { static_cast< BasicBuffer_t< elem >* >( base::rdbuf() )->SetPriority( newPriority_ ); }
			virtual void SetDefaultPriority( SettingsType newDefault_ ) { static_cast< BasicBuffer_t< elem >* >( base::rdbuf() )->SetDefaultPriority( newDefault_ ); }
			virtual void SetFilter( SettingsType newCap_ ) { static_cast< BasicBuffer_t< elem >* >( base::rdbuf() )->SetFilter( newCap_ ); }
			virtual SettingsType GetEnable() { return static_cast< BasicBuffer_t< elem > * >( base::rdbuf() )->GetEnable(); }
			virtual SettingsType GetPriority() { return static_cast< BasicBuffer_t< elem > * >( base::rdbuf() )->GetPriority(); }
			virtual SettingsType GetDefaultPriority() { return static_cast< BasicBuffer_t< elem >* >( base::rdbuf() )->GetDefaultPriority(); }
			virtual SettingsType GetFilter() { return static_cast< BasicBuffer_t< elem > * >( base::rdbuf() )->GetFilter(); }
			void SetIsChannelTarget( bool isShared_ ) { 
				BasicBuffer_t< elem >* buf = static_cast< BasicBuffer_t< elem > * >( base::rdbuf() );
				buf->SetIsChannelTarget( isShared_ );
			}
			bool GetIsChannelTarget() { return static_cast< BasicBuffer_t< elem > * >( base::rdbuf() )->GetIsChannelTarget(); }

			// access functions when the stream is a shared target
			void Lock() { m_lock.lock(); }
			void Unlock() { m_lock.unlock(); }
			bool TryLock() { return m_lock.try_lock(); }

		protected:
			std::mutex m_lock;
			BasicStream_t() = delete;
			BasicStream_t( BasicStream_t const & other_ ) = delete;
			BasicStream_t operator=( BasicStream_t const & other_ ) = delete;
		};

		// default BasicStream_t derivative performs integral narrowing / widening of non-native character pointers
		template< typename ELEM_ >
		class Stream_t : public BasicStream_t< ELEM_ >
		{
		public:
			using elem = ELEM_;
			Stream_t( BasicBuffer_t< elem > * buffer_ )
				: BasicStream_t< elem >( buffer_ )
			{}
			virtual ~Stream_t() {}
		};

		// specialisation whose helpers convert non-native strings to the UTF format most appropriate to the stream width
		template< typename ELEM_ >
		class ConvertingStream_t : public BasicStream_t< ELEM_  >
		{
		public:
			using elem = ELEM_;
			ConvertingStream_t( BasicBuffer_t< elem >* buffer_ )
				: BasicStream_t< elem >( buffer_ )
			{}
			virtual ~ConvertingStream_t() {}
		};

		//////////////////////////////////////////////////////////////////////////
		/// BasicBuffer
		//////////////////////////////////////////////////////////////////////////

		template < class ELEM_ >
		class BasicBuffer_t : public std::basic_stringbuf< ELEM_, std::char_traits< ELEM_ >, std::allocator< ELEM_ > >
		{
		public:
			using elem = ELEM_;
			using base = std::basic_stringbuf< elem, std::char_traits< elem >, std::allocator< ELEM_ > >;

			BasicBuffer_t( OutputTarget& target_, StreamSettings& initSettings_ = GetDefaultChannelSettings(), OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
				: std::basic_stringbuf< ELEM_, std::char_traits< ELEM_ >, std::allocator< ELEM_ > >( )
				, m_outputTarget( target_ )
				, m_stamp( stamp_ )
				, m_isChannelTarget( false )
			{
				ReserveStamp();
				m_settings.Enable( initSettings_.GetEnable() );
				m_settings.SetPriority( initSettings_.GetPriority() );
				m_settings.SetDefaultPriority( initSettings_.GetDefaultPriority() );
				m_settings.SetFilter( initSettings_.GetFilter() );
			}

			virtual void Enable( SettingsType enable_ ) { m_settings.Enable( enable_ ); }
			virtual void SetPriority( SettingsType newPriority_ ) { m_settings.SetPriority( newPriority_ ); }
			virtual void SetDefaultPriority( SettingsType newDefault_ ) { m_settings.SetDefaultPriority( newDefault_ ); }
			virtual void SetFilter( SettingsType newCap_ ) { m_settings.SetFilter( newCap_ ); }
			virtual SettingsType GetEnable() { return m_settings.GetEnable(); }
			virtual SettingsType GetPriority() { return m_settings.GetPriority(); }
			virtual SettingsType GetDefaultPriority() { return m_settings.GetDefaultPriority(); }
			virtual SettingsType GetFilter() { return m_settings.GetFilter(); }

			StreamSettings& GetSettings() {
				return m_settings;
			}
			void SetIsChannelTarget( bool isShared_ )
			{
				m_isChannelTarget.store( isShared_, std::memory_order_release );
			}
			bool GetIsChannelTarget() {
				return m_isChannelTarget.load( std::memory_order_acquire );
			}

			// write timestamp to buffer then reset put position to start of buffer
			void ReserveStamp()
			{
				auto len = m_stamp.GetMaxLength();
				for ( auto i = 0; i < len; ++i )
					base::sputc( static_cast< elem >( 'B' ) );
				
				m_cToNext = len;
			}
			virtual ~BasicBuffer_t() {}
			void SetOriginalBufferStart()
			{
				base::setp( base::pbase(), base::epptr() );
				base::pbump( -static_cast< int >( base::pptr() - base::pbase() ) );
				m_cToNext = 0;
			}

			virtual int flush( bool bIsStreamFlush = false )
			{	
				auto bIsTarget = GetIsChannelTarget();
				auto bWriteStamp = !bIsTarget;
				int maxLength = m_stamp.GetMaxLength();
				int numToOutput = static_cast< uint32_t >( base::pptr() - base::pbase() );

				if ( m_settings.CanBeOutput() )
				{
					if ( bIsStreamFlush )
					{
						int stampLength = 0;
						int offset = 0;
						
						if ( !bIsTarget )
						{
							stampLength = m_stamp.GetLength();
							offset = maxLength - stampLength;
							numToOutput -= maxLength - stampLength;
						}

						int numBytes = numToOutput * sizeof( elem );

						base::sputc( 0 );

						if ( bIsStreamFlush )
						{
							if ( numToOutput == ( maxLength - offset ) + 1 )
							{
								elem lastChar = *( base::pptr() - 2 );
								// don't timestamp lines with carriage return only
								if ( lastChar == '\n' )
								{
									offset = maxLength;
									numToOutput = 1;
									numBytes = sizeof( elem );
									bWriteStamp = false;
								}
							}
							if( bWriteStamp )
								m_stamp.WriteStamp( base::pbase() + offset );
							m_outputTarget.Output( base::pbase() + offset, numToOutput++, numBytes );
						}
					}
					else
						m_cToNext = base::pptr() - base::pbase();	// update count but don't output
				}
				base::pbump( m_cToNext - static_cast< int >( base::pptr() - base::pbase() ) );
				return 0;
			}
			virtual int sync() override
			{
				m_cToNext = GetIsChannelTarget() ? 0 : m_stamp.GetMaxLength();
				flush( true );
				// reset priority level to default priority following each flush of the stream
				m_settings.SetPriority( m_settings.GetDefaultPriority() );
				return 0;
			}
			OutputTarget& GetOutputTarget() {
				return m_outputTarget;
			}
		protected:
			int m_cToNext;
			OutputStamp& m_stamp;
		private:
			OutputTarget& m_outputTarget;
			StreamSettings m_settings;
			std::atomic< bool > m_isChannelTarget;
			BasicBuffer_t( BasicBuffer_t const & other_ ) = delete;
			BasicBuffer_t operator=( BasicBuffer_t const & other_ ) = delete;
		};

		//////////////////////////////////////////////////////////////////////////
		/// OutputBuffer - flushes to an OutputTarget<> template class
		//////////////////////////////////////////////////////////////////////////

		template < typename TARGET_ >
		class OutputBuffer_t : public BasicBuffer_t< typename TARGET_::elem >
		{
		public:
			using elem = typename TARGET_::elem;
			using base = BasicBuffer_t< elem >;
			OutputBuffer_t( char const * const fileName_ = nullptr, StreamSettings& initialSettings_ = GetDefaultChannelSettings(), OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
				: BasicBuffer_t< elem >( m_outputTarget, initialSettings_, stamp_ )
				, m_outputTarget( fileName_ )
			{
			}
			virtual ~OutputBuffer_t() = default;
			OutputTarget& GetOutputTarget() { return m_outputTarget; }
		private:
			OutputBuffer_t( OutputBuffer_t const & rhs_ ) = delete;
			OutputBuffer_t & operator = ( OutputBuffer_t const & rhs_ ) = delete;
			TARGET_ m_outputTarget;
		};

		template < typename ELEM_ >
		class NoOutput_t : public BasicBuffer_t< ELEM_ >
		{
		public:
			using elem = ELEM_;
			using base = BasicBuffer_t< elem >;
			NoOutput_t( StreamSettings& initialSettings_ = GetDefaultChannelSettings(), OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
				: BasicBuffer_t< elem >( m_dummyTarget, initialSettings_, stamp_ )
			{
			}
			virtual int flush( bool bIsStreamFlush = false ) override
			{
				return 0;
			}
			virtual int sync() override
			{
				// just empty
				base::pbump( -static_cast< int >( base::pptr() - base::pbase() ) );
				return 0;
			}
			virtual ~NoOutput_t() = default;

		private:
			NoOutput_t( NoOutput_t const& rhs_ ) = delete;
			NoOutput_t& operator = ( NoOutput_t const& rhs_ ) = delete;
			OutputTarget m_dummyTarget;
		};


		//////////////////////////////////////////////////////////////////////////
		/// OutputStream class. Needs base class and OutputTarget via template params
		/// Base class will depend on whether UTF conversions are required
		//////////////////////////////////////////////////////////////////////////

		template< typename STREAMBASE_ >
		class OutputStream_t : public STREAMBASE_
		{
		public:
			using elem = typename STREAMBASE_::elem;
			OutputStream_t( BasicBuffer_t< elem >* buffer_ )
				: STREAMBASE_( buffer_ )
				, m_buffer( buffer_ )
			{}
			
			virtual ~OutputStream_t()
			{
				if ( STREAMBASE_::GetIsChannelTarget() )
				{
					// ensure OutputChannels know we have been destroyed
					size_t index = GetIndexFromPointer( this );
					GetMasterChannelMutex().lock();
					GetStreamInitFlags()[ index ] = 0;
					GetSharedStreamArray()[ index ] = nullptr;
					GetMasterChannelMutex().unlock();
				}
			}
			OutputTarget& GetOutputTarget() {
				return m_buffer->GetOutputTarget();
			}
		protected:
			BasicBuffer_t< elem >* m_buffer;
			OutputStream_t( OutputStream_t const & other_ ) = delete;
			OutputStream_t operator=( OutputStream_t const & other_ ) = delete;
		};


		//////////////////////////////////////////////////////////////////////////
		/// A specialisation of OutputStream which captures another stream's buffer (e.g. std::out) and diverts it to an OutputTarget
		//////////////////////////////////////////////////////////////////////////

		template< typename STREAMBASE_ >
		class OutputStreamCapture_t : public OutputStream_t< STREAMBASE_ >
		{
		public:
			using elem = typename STREAMBASE_::elem;
			OutputStreamCapture_t( StreamSettings * initialSettings_, char const * const fileName_ = nullptr, std::basic_ostream< elem > * stream_ = nullptr )
				: OutputStream_t< STREAMBASE_ >( initialSettings_, fileName_ )
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
			OutputStream_t< STREAMBASE_ > * m_prevStream;
			std::basic_stringbuf< elem, std::char_traits< elem > > * m_prevStreamBuf;

			OutputStreamCapture_t() = delete;
			OutputStreamCapture_t( OutputStreamCapture_t const & other_ ) = delete;
			OutputStreamCapture_t operator=( OutputStreamCapture_t const & other_ ) = delete;
		};

		template< typename T_, typename U_ = Stream_t< typename T_::elem  > >
		class OutputStreamComplete_t : public OutputStream_t< U_ >
		{
		public:
			OutputStreamComplete_t( char const* const initString_ = nullptr, StreamSettings& settings_ = GetDefaultChannelSettings(), OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
				: OutputStream_t< U_ >( &m_buffer )
				, m_buffer( initString_, settings_, stamp_ )
			{
			}
			virtual ~OutputStreamComplete_t() = default;
		private:
			OutputBuffer_t< T_ > m_buffer;
		};

		template< typename T_, typename U_ = Stream_t< typename T_::elem  > >
		class NoOutputStreamComplete_t : public OutputStream_t< U_ >
		{
		public:
			NoOutputStreamComplete_t( StreamSettings& settings_ = GetDefaultChannelSettings(), OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
				: OutputStream_t< U_ >( &m_buffer )
				, m_buffer( settings_, stamp_ )
			{
			}
			virtual ~NoOutputStreamComplete_t() = default;
		private:
			 T_ m_buffer;
		};

		//////////////////////////////////////////////////////////////////////////
		/// OutputStream and OutputChannel manipulator functors with proxy template for other stream types
		//////////////////////////////////////////////////////////////////////////

		struct Enable
		{
			explicit Enable( int val_ )
				: payload_( val_ )
			{}
			template< typename T_ >
			std::basic_ostream< T_ >& operator()( std::basic_ostream< T_ >& strm_ ) const	// ignored with incompatible stream type
			{
				return strm_;
			}
			template< typename T_ >
			ConvertingStream_t< T_ >& operator()( ConvertingStream_t< T_ >& strm_ ) const
			{
				BasicBuffer_t< T_ >* ptr = static_cast< BasicBuffer_t< T_ > * >( strm_.rdbuf() );
				ptr->flush();
				ptr->Enable( payload_ );
				return strm_;
			}
			template< typename T_ >
			Stream_t< T_ >& operator()( Stream_t< T_ >& strm_ ) const
			{
				BasicBuffer_t< T_ >* ptr = static_cast< BasicBuffer_t< T_ > * >( strm_.rdbuf() );
				ptr->flush();
				ptr->Enable( payload_ );
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
			std::basic_ostream< T_ >& operator()( std::basic_ostream< T_ >& strm_ ) const
			{
				return strm_;
			}
			template< typename T_ >
			ConvertingStream_t< T_ >& operator()( ConvertingStream_t< T_ >& strm_ ) const
			{
				BasicBuffer_t< T_ > * ptr = static_cast< BasicBuffer_t< T_ > * >( strm_.rdbuf() );
				ptr->flush();
				ptr->SetPriority( payload_ );
				return strm_;
			}
			template< typename T_ >
			Stream_t< T_ >& operator()( Stream_t< T_ >& strm_ ) const
			{
				BasicBuffer_t< T_ >* ptr = static_cast< BasicBuffer_t< T_ > * >( strm_.rdbuf() );
				ptr->flush();
				ptr->SetPriority( payload_ );
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
			std::basic_ostream< T_ >& operator()( std::basic_ostream< T_ >& strm_ ) const
			{
				return strm_;
			}
			template< typename T_ >
			ConvertingStream_t< T_ >& operator()( ConvertingStream_t< T_ >& strm_ ) const
			{
				BasicBuffer_t< T_ >* ptr = static_cast< BasicBuffer_t< T_ > * >( strm_.rdbuf() );
				ptr->flush();
				ptr->SetDefaultPriority( payload_ );
				ptr->SetPriority( payload_ );
				return strm_;
			}
			template< typename T_ >
			Stream_t< T_ >& operator()( Stream_t< T_ >& strm_ ) const
			{
				BasicBuffer_t< T_ >* ptr = static_cast< BasicBuffer_t< T_ > * >( strm_.rdbuf() );
				ptr->flush();
				ptr->SetDefaultPriority( payload_ );
				ptr->SetPriority( payload_ );
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
			std::basic_ostream< T_ >& operator()( std::basic_ostream< T_ >& strm_ ) const
			{
				return strm_;
			}
			template< typename T_ >
			ConvertingStream_t< T_ >& operator()( ConvertingStream_t< T_ >& strm_ ) const
			{
				BasicBuffer_t< T_ >* ptr = static_cast< BasicBuffer_t< T_ > * >( strm_.rdbuf() );
				ptr->flush();
				ptr->SetFilter( payload_ );
				return strm_;
			}
			template< typename T_ >
			Stream_t< T_ >& operator()( Stream_t< T_ >& strm_ ) const
			{
				BasicBuffer_t< T_ >* ptr = static_cast< BasicBuffer_t< T_ > * >( strm_.rdbuf() );
				ptr->flush();
				ptr->SetFilter( payload_ );
				return strm_;
			}
			int payload_;
		};

		// custom streams::endl and forwarder
		// std::endl won't work correctly with template forwarders
		// so we get a basic_ostream on its return - this isn't a problem at the end of a chain
		// but does mean 'blah << blah << std::endl << Priority(4) << more blah' etc. won't invoke the custom manips
		// Therefore, prefer this streams::endl or just a plain '\n' if you intend to use other OutputStreams manipulators
		// after a carriage return

		template< typename T_ >
		inline Stream_t< T_ >& endl( Stream_t< T_ >& strm_ )
		{
			std::basic_ostream< T_, std::char_traits< T_ > >& baseCast = static_cast< std::basic_ostream< T_, std::char_traits< T_ > >& >( strm_ );
			endl( baseCast );
			return strm_;
		}

		template< typename T_ >
		inline Stream_t< T_ >& operator << ( Stream_t< T_ >& strm_, Stream_t< T_ >& ( *fnc )( Stream_t< T_ >& ) )
		{
			fnc( strm_ );
			return strm_;
		}
		
		//////////////////////////////////////////////////////////////////////////
		// manipulator catchers
		//////////////////////////////////////////////////////////////////////////


		template< typename T_ >
		std::basic_ostream< T_ >& operator <<( std::basic_ostream< T_ >& stream_, Enable const& obj_ )
		{
			return obj_( stream_ );
		}
		template< typename T_ >
		Stream_t< T_ >& operator << ( Stream_t< T_ >& stream_, Enable const& obj_ )
		{
			return obj_( stream_ );
		}
		template< typename T_ >
		ConvertingStream_t< T_ >& operator << ( ConvertingStream_t< T_ >& stream_, Enable const& obj_ )
		{
			return obj_( stream_ );
		}
		template< typename T_ >
		std::basic_ostream< T_ >& operator << ( std::basic_ostream< T_ >& stream_, DefaultPriority const& obj_ )
		{
			return obj_( stream_ );
		}
		template< typename T_ >
		Stream_t< T_ >& operator << ( Stream_t< T_ >& stream_, DefaultPriority const& obj_ )
		{
			return obj_( stream_ );
		}
		template< typename T_ >
		ConvertingStream_t< T_ >& operator << ( ConvertingStream_t< T_ >& stream_, DefaultPriority const& obj_ )
		{
			return obj_( stream_ );
		}
		template< typename T_ >
		std::basic_ostream< T_ >& operator <<( std::basic_ostream< T_ >& stream_, Filter const& obj_ )
		{
			return obj_( stream_ );
		}
		template< typename T_ >
		Stream_t< T_ >& operator << ( Stream_t< T_ >& stream_, Filter const& obj_ )
		{
			return obj_( stream_ );
		}
		template< typename T_ >
		ConvertingStream_t< T_ >& operator << ( ConvertingStream_t< T_ >& stream_, Filter const& obj_ )
		{
			return obj_( stream_ );
		}
		template< typename T_ >
		std::basic_ostream< T_ >& operator <<( std::basic_ostream< T_ >& stream_, Priority const& obj_ )
		{
			return obj_( stream_ );
		}
		template< typename T_ >
		Stream_t< T_ >& operator << ( Stream_t< T_ >& stream_, Priority const& obj_ )
		{
			return obj_( stream_ );
		}
		template< typename T_ >
		ConvertingStream_t< T_ >& operator << ( ConvertingStream_t< T_ >& stream_, Priority const& obj_ )
		{
			return obj_( stream_ );
		}

		//////////////////////////////////////////////////////////////////////////
		// generic catchers and forwarders for most std::manipulators
		//////////////////////////////////////////////////////////////////////////

		template< typename T_, typename U_ >
		Stream_t< T_ >& operator << ( Stream_t< T_ >& stream_, U_ const& obj_ )
		{

			std::basic_ostream< T_, std::char_traits< T_ > >& baseCast = static_cast< std::basic_ostream< T_, std::char_traits< T_ > >& >( stream_ );
			baseCast << obj_;
			return stream_;
		}

		template< typename T_, typename U_ >
		ConvertingStream_t< T_ >& operator << ( ConvertingStream_t< T_ >& stream_, U_ const& obj_ )
		{
			std::basic_ostream< T_, std::char_traits< T_ > >& baseCast = static_cast< std::basic_ostream< T_, std::char_traits< T_ > >& >( stream_ );
			baseCast << obj_;
			return stream_;
		}
		
		//////////////////////////////////////////////////////////////////////////
		/// Stream and Channel string pointer handler for non-converting streams
		/// (performs integral narrowing/widening)
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
		/// We have to make assumptions of course - see function descriptions.
		/// Also note that code assumes little endian data and target at present.
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
		/// Operator << C-string and std::basic_string pointer catcher specialisations
		/// These forward to the integral C-string conversion function
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

		// string handlers
		template< typename ELEM_, typename T_ >
		Stream_t< ELEM_ >& operator <<( Stream_t< ELEM_ >& stream_, std::basic_string< T_ >const& str_ )
		{
			return operator <<( stream_, str_.c_str() );
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

		// conversion stream support for string types
		template< typename ELEM_, typename T_ >
		ConvertingStream_t< ELEM_ >& operator <<( ConvertingStream_t< ELEM_ >& stream_, std::basic_string< T_ >const& str_ )
		{
			return operator <<( stream_, str_.c_str() );
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

#else	// OUTPUT_STREAM_STRIP is being used - alias to / base all types from NullStream

		// Buffer object definitions
		template< typename T_ >
		using BasicBuffer_t = NullStream_t< T_ >;

		template< typename T_ >
		class OutputBuffer_t : public BasicBuffer_t< typename T_::elem >
		{
		public:
			OutputBuffer_t( char const* const fileName_ = nullptr, StreamSettings& initialSettings_ = GetDefaultChannelSettings(), OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
			{}
		};

		template< typename T_ >
		using NoOutput_t = BasicBuffer_t< T_ >;

		// Stream object definitions
		template< typename T_ >
		using BasicStream_t = NullStream_t< T_ >;

		template< typename T_ >
		class Stream_t : public NullStream_t< T_ >
		{
		public:
			using elem = T_;
		};

		template< typename T_ >
		class ConvertingStream_t : public NullStream_t< T_ >
		{
		public:
			using elem = T_;
		};

		template< typename T_ >
		class OutputStream_t : public T_
		{
		public:
			inline OutputStream_t( void* pBuffer_ )
			{}
		};
		
		// manipulators

		struct Enable
		{
			explicit Enable( int val_ )
			{
			}
			template< typename T_ >
			T_& operator()( T_& strm_ )
			{
				return strm_;
			}
		};

		struct Priority
		{
			explicit Priority( int val_ )
			{
			}
			template< typename T_ >
			T_& operator()( T_& strm_ )
			{
				return strm_;
			}
		};

		struct DefaultPriority
		{
			explicit DefaultPriority( int val_ )
			{
			}
			template< typename T_ >
			T_& operator()( T_& strm_ )
			{
				return strm_;
			}
		};

		struct Filter
		{
			explicit Filter( int val_ )
			{
			}
			template< typename T_ >
			T_& operator()( T_& strm_ )
			{
				return strm_;
			}
		};

		// NullStream function object (manipulator) forwarders
		template< typename T_ >
		NullStream_t<T_>& operator << ( NullStream_t<T_>& stream_, Enable const& obj_ )
		{
			return stream_;
		}
		template< typename T_ >
		NullStream_t<T_>& operator << ( NullStream_t<T_>& stream_, Priority const& obj_ )
		{
			return stream_;
		}
		template< typename T_ >
		NullStream_t<T_>& operator << ( NullStream_t<T_>& stream_, DefaultPriority const& obj_ )
		{
			return  stream_;
		}
		template< typename T_ >
		NullStream_t<T_>& operator << ( NullStream_t<T_>& stream_, Filter const& obj_ )
		{
			return stream_;
		}

		template< typename T_ >
		NullStream_t<T_>& endl( NullStream_t<T_>& strm_ )
		{
			return strm_;
		}	

		template< typename T_, typename U_ = Stream_t< typename T_::elem > >
		class OutputStreamComplete_t : public U_
		{
		public:
			OutputStreamComplete_t( char const* const initString_ = nullptr, StreamSettings& settings_ = GetDefaultChannelSettings(), OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
			{
			}
		};

		template< typename T_, typename U_ = Stream_t< typename T_::elem  > >
		class NoOutputStreamComplete_t : public U_
		{
		public:
			NoOutputStreamComplete_t( StreamSettings& settings_ = GetDefaultChannelSettings(), OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
			{
			}
		};
#endif 	// #if !defined( OUTPUT_STREAM_STRIP )
		
	} // namespace streams
} // namespace mbp
#endif // #ifndef OutputStreams_DEFINED_31_12_2014


