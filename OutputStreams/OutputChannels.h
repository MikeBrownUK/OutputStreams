//////////////////////////////////////////////////////////////////////////
/// ©Mike Brown, 2014-2026
/// https://www.mikebrown.co.uk
///
///	Filename: 	OutputChannels.h
///	Created:	31/7/2019
///	Author:		Mike Brown
///	
///	Description: Declaration of OutputChannel objects
///
//////////////////////////////////////////////////////////////////////////

#ifndef OutputChannels_DEFINED_31_07_2019
#define OutputChannels_DEFINED_31_07_2019

#include <vector>
#if defined(__linux)
#include <cstring>
#endif // #if defined(__linux)
#include "OutputStreams.h"
#include "assert.h"

namespace mbp
{
	namespace streams
	{
		// arbitrary number of OutputChannels - required for channel settings which have to be accessible to all active threads
		auto constexpr kMaxOutputChannels = 64;
		// the maximum number of shared OutputStreams 
		auto constexpr kMaxSharedStreams = 32;
		// a prime number that's close to but larger than KMaxSharedStreams to use for pointer hashing and array capacity
		auto constexpr kPrime = 37;

		// array of priority and filter settings for each channel, indexed by Channel ID
		extern StreamSettings g_AllChannelSettings[ kMaxOutputChannels ];
		extern uint8_t * GetChannelInitFlags();
		// array of shared stream pointers and initialisation flags used to prevent new channels from performing reinitialisation when attaching to the same one
		extern void ** GetSharedStreamArray();
		extern uint8_t * GetStreamInitFlags();
		// mutex protecting the above
		extern std::mutex & GetMasterChannelMutex();
		// hash and probe function
		size_t GetIndexFromPointer( void * ptr_ );

		template < typename ELEM_, bool MULTITHREAD_ = true >
		class ChannelBuffer_t;

#if !defined( OUTPUT_STREAM_STRIP )

		// An OutputChannel uses a Channel ID and attaches to one or more OutputStreams, allowing per-channel filtering of output
		template< typename STREAMBASE_ >
		class OutputChannel_t : public STREAMBASE_
		{
		public:
			using elem = typename STREAMBASE_::elem;
			using base = STREAMBASE_;

			OutputChannel_t( BasicBuffer_t< elem >* pBuffer_, int channelID_, std::vector< BasicStream_t< elem > * > const& streams_, bool isMultiThreadChannel_ = true, StreamSettings& initSettings_ = GetDefaultChannelSettings(), bool bCleanupBuffer = false )
				: STREAMBASE_( pBuffer_ )
				, m_channelId( channelID_ )
				, m_sharedStreams( streams_ )
				, m_bCleanup( bCleanupBuffer )
			{
				assert( channelID_ < kMaxOutputChannels );
				GetMasterChannelMutex().lock();
				// check to see if channel ID's settings have been initialised (only the first channel constructed with an ID gets to do this)
				if ( 0 == GetChannelInitFlags()[ channelID_ ]++ )
				{
					// forward initial settings to channel ID settings
					g_AllChannelSettings[ channelID_ ].Enable( initSettings_.GetEnable() );
					g_AllChannelSettings[ channelID_ ].SetPriority( initSettings_.GetPriority() );
					g_AllChannelSettings[ channelID_ ].SetDefaultPriority( initSettings_.GetDefaultPriority() );
					g_AllChannelSettings[ channelID_ ].SetFilter( initSettings_.GetFilter() );
				}
				for ( auto& i : m_sharedStreams )
				{
					size_t index = GetIndexFromPointer( i );
					GetSharedStreamArray()[ index ] = i;
				}
				GetMasterChannelMutex().unlock();
			}
			virtual ~OutputChannel_t()
			{
				size_t index;
				if( m_bCleanup )
					delete base::rdbuf();
				{
					GetMasterChannelMutex().lock();
					--GetChannelInitFlags()[ m_channelId ];
					for ( auto *& i : m_sharedStreams )
					{
						index = GetIndexFromPointer( i );
						if ( 1 == GetStreamInitFlags()[ index ]-- )
						{
							GetSharedStreamArray()[ index ] = nullptr;
							i->SetIsChannelTarget( false );
							BasicBuffer_t< elem > * buff = static_cast< BasicBuffer_t< elem > * >( i->rdbuf() );
							buff->ReserveStamp();
						}
					}
					GetMasterChannelMutex().unlock();
				}
			}
			int const GetChannelId() const { return m_channelId; }
		private:
			bool m_bCleanup;
			int const m_channelId;
			std::vector < BasicStream_t< elem > * > m_sharedStreams;
			OutputChannel_t() = delete;
			OutputChannel_t( OutputChannel_t const & other_ ) = delete;
			OutputChannel_t operator=( OutputChannel_t const & other_ ) = delete;
		};

		// ChannelBuffer is an OutputChannel's buffer specialisation
		template < class ELEM_, bool MULTITHREAD_ >
		class ChannelBuffer_t : public BasicBuffer_t< ELEM_ >
		{
		protected:
			using elem = ELEM_;
			using traits = std::char_traits < elem >;
			using base = BasicBuffer_t< ELEM_ >;
		public:
			ChannelBuffer_t( int channelID_, std::vector< BasicStream_t< elem > * >const & shared_, OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
				: BasicBuffer_t< ELEM_ >( m_dummyTarget )
				, m_bInitialised( false )
				, m_channelID( channelID_ )
				, m_stamp( stamp_ )
			{
				for( auto &i : shared_ )
				{
					m_streamIndices.push_back( GetIndexFromPointer( i ) );
				}
				auto len = m_stamp.GetMaxLength();

				// force initial allocation
				for ( auto i = 0; i < len; ++i )
				{
					base::sputc( 'C' );
				}
				base::m_cToNext = m_stamp.GetMaxLength();
				
			}
			virtual ~ChannelBuffer_t()	
			{
				m_streamIndices.clear();
			}

			// enable channel and filter functions for the shared stream (all threads)
			virtual void Enable( SettingsType enable_ ) override { g_AllChannelSettings[ m_channelID ].Enable( enable_ ); }
			virtual SettingsType GetEnable() override { return g_AllChannelSettings[ m_channelID ].GetEnable(); }
			virtual void SetPriority( SettingsType newPriority_ ) override { g_AllChannelSettings[ m_channelID ].SetPriority( newPriority_ ); }
			virtual SettingsType GetPriority() override { return g_AllChannelSettings[ m_channelID ].GetPriority(); }
			virtual void SetDefaultPriority( SettingsType newDefault_ ) override
			{
				g_AllChannelSettings[ m_channelID ].SetDefaultPriority( newDefault_ ); g_AllChannelSettings[ m_channelID ].SetPriority( newDefault_ );
			}
			virtual SettingsType GetDefaultPriority() override { return g_AllChannelSettings[ m_channelID ].GetDefaultPriority(); }
			virtual void SetFilter( SettingsType newFilter_ ) override { g_AllChannelSettings[ m_channelID ].SetFilter( newFilter_ ); }
			virtual SettingsType GetFilter() override { return g_AllChannelSettings[ m_channelID ].GetFilter(); }

		protected:
			void PerformInit()
			{
				for ( auto i : m_streamIndices )
				{
					GetMasterChannelMutex().lock();
					if ( 0 == GetStreamInitFlags()[ i ]++ )
					{
						BasicStream_t < elem >* strm = reinterpret_cast< BasicStream_t < elem > * >( GetSharedStreamArray()[ i ] );
						strm->SetIsChannelTarget( true );
						BasicBuffer_t< elem >* buff = static_cast< BasicBuffer_t< elem > * >( strm->rdbuf() );
						buff->SetOriginalBufferStart();

					}
					GetMasterChannelMutex().unlock();
				}
				m_bInitialised = true;	
			}

			virtual int flush( bool bIsStreamFlush = false ) override
			{
				bool bWriteStamp = true;
				BasicStream_t< elem >* strm;
				auto maxLength = m_stamp.GetMaxLength();
				auto stampLength = 0;
				auto offset = 0;
				auto numToOutput = base::pptr() - base::pbase();	// this will hold total characters to output including prefix
				auto numSentToStream = numToOutput - maxLength;		// number of characters sent to stream via client

				uint8_t writesComplete[ kMaxSharedStreams ];

				if ( numSentToStream && g_AllChannelSettings[ m_channelID ].CanBeOutput() )
				{
					if ( bIsStreamFlush )
					{
						m_stamp.Lock();
						stampLength = m_stamp.GetLength();
						m_stamp.Unlock();
						auto offsetInBuffer = maxLength - stampLength;		// position in buffer for forwarding		
						numToOutput -= maxLength - stampLength;				// finalised output count

						for ( auto i = 0u; i < m_streamIndices.size(); ++i )
							writesComplete[ i ] = 0;

						int done, j;

						do
						{
							j = 0;
							done = 1;
							for ( auto i : m_streamIndices )
							{
								if ( !writesComplete[ j ] )
								{
									strm = reinterpret_cast< BasicStream_t < elem > * >( GetSharedStreamArray()[ i ] );
									if ( strm && reinterpret_cast< BasicBuffer_t< elem > * >( strm->rdbuf() )->GetSettings().CanBeOutput() )
									{

										if ( numSentToStream == 1 )
										{
											elem lastChar = *( base::pptr() - 1 );
											// don't timestamp lines with carriage return only
											if ( lastChar == '\n' )
											{
												offsetInBuffer = maxLength;
												numToOutput = 1;
												bWriteStamp = false;
												// and abort all attached stream writes
												done = 1;
												break;
											}
										}
										if ( strm->TryLock() )
										{
											if ( bWriteStamp )
												m_stamp.WriteStamp( base::pbase() + offsetInBuffer );
											strm->write( base::pbase() + offsetInBuffer, numToOutput );
											strm->flush();
											strm->Unlock();
											++writesComplete[ j ];
										}
									}
								}
							}
						} while ( !done );
					}
					else
					{
						base::m_cToNext = base::pptr() - base::pbase();	// update buffer count but don't output at all
					}
				}
				base::pbump( base::m_cToNext - static_cast< int >( base::pptr() - base::pbase() ) );
				return 0;
			}

			virtual int sync() override
			{
				if ( !m_bInitialised )
					PerformInit();
				base::m_cToNext = m_stamp.GetMaxLength();
				flush( true );
				g_AllChannelSettings[ m_channelID ].SetPriority( g_AllChannelSettings[ m_channelID ].GetDefaultPriority() );
				return 0;
			}

			bool m_bInitialised;
			OutputTarget m_dummyTarget;
			int m_channelID;
			OutputStamp& m_stamp;
			std::vector< size_t > m_streamIndices;
			ChannelBuffer_t() = delete;
			ChannelBuffer_t( ChannelBuffer_t const & rhs_ ) = delete;
			ChannelBuffer_t & operator = ( ChannelBuffer_t const & rhs_ ) = delete;
		};

		// ChannelBuffer specialisation for single-thread use
		template< typename ELEM_ >
		class ChannelBuffer_t< ELEM_, false > : public ChannelBuffer_t< ELEM_, true  >
		{
		public:
			using elem = ELEM_;
			using base = ChannelBuffer_t< ELEM_, true >;

			ChannelBuffer_t( int channelID_, std::vector< BasicStream_t< elem >* >const& shared_, OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
				: ChannelBuffer_t< elem, true >( channelID_, shared_, stamp_ )
			{
			}
			virtual ~ChannelBuffer_t() {}
			virtual int flush( bool bIsStreamFlush = false ) override
			{	
				bool bWriteStamp = true;
				BasicStream_t< elem >* strm;
				auto maxLength = base::m_stamp.GetMaxLength();
				auto stampLength = base::m_stamp.GetLength();
				auto offset = maxLength - stampLength;
				auto numSentToStream = base::pptr() - base::pbase() - maxLength;
				auto numToOutput = numSentToStream + stampLength;
				
				if ( g_AllChannelSettings[ base::m_channelID ].CanBeOutput() )
				{
					if ( bIsStreamFlush == false )
						base::m_cToNext = base::pptr() - base::pbase();
					else
					{
						for ( auto i : base::m_streamIndices )
						{
							strm = reinterpret_cast< BasicStream_t < elem > * >( GetSharedStreamArray()[ i ] );
							if ( ( reinterpret_cast< BasicBuffer_t< elem > * >( strm->rdbuf() ) )->GetSettings().CanBeOutput() )
							{
								if ( bIsStreamFlush )
								{
									if ( numToOutput == ( maxLength - offset ) + 1 )
									{
										elem lastChar = *( base::pptr() - 1 );
										// don't timestamp lines with carriage return only
										if ( lastChar == '\n' )
										{
											offset = maxLength;
											numToOutput = 1;
											bWriteStamp = false;
										}
									}
									if( bWriteStamp )
										base::m_stamp.WriteStamp( base::pbase() + offset );
									strm->write( base::pbase() + offset, numToOutput );
									strm->flush();
									g_AllChannelSettings[ base::m_channelID ].SetPriority( g_AllChannelSettings[ base::m_channelID ].GetDefaultPriority() );
								}
							}
						}
					}
				}
				base::pbump( base::m_cToNext - static_cast< int >( base::pptr() - base::pbase() ) );
				
				return 0;
			}
		};

		template< typename T_, bool MULTITHREAD_ = true >
		class OutputChannelComplete_t : public OutputChannel_t< T_ >
		{
		public:
			OutputChannelComplete_t( int channelID_, std::vector< BasicStream_t< typename T_::elem >* > const& streams_, StreamSettings& initSettings_ = GetDefaultChannelSettings(), OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
				: OutputChannel_t< T_ >( &m_buffer, channelID_, streams_, MULTITHREAD_, initSettings_ )
				, m_buffer( channelID_, streams_, stamp_ )
			{
			}
			virtual ~OutputChannelComplete_t() = default;
		private:
			ChannelBuffer_t< typename T_::elem, MULTITHREAD_ > m_buffer;
		};
#else

		template< typename T_, bool MULTITHREAD_ >
		class ChannelBuffer_t : public NullStream_t< T_ >
		{
		public:
			ChannelBuffer_t( int channelID_, std::vector< BasicStream_t< T_ >* >const& shared_, OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
			{
			}
		};

		template< typename T_ >
		class OutputChannel_t : public OutputStream_t< T_ >
		{
		public:
			OutputChannel_t( ChannelBuffer_t< typename T_::elem >* pBuffer_, int channelID_, std::vector< BasicStream_t< typename T_::elem >* > const& streams_, bool isMultiThreadChannel_ = true, StreamSettings& initSettings_ = GetDefaultChannelSettings(), bool bCleanupBuffer = false )
				: OutputStream_t< T_ >( pBuffer_ )
			{
			}
		};
		
		template< typename T_, bool MULTITHREAD_ >
		class OutputChannelComplete_t : public T_
		{
		public:
			inline OutputChannelComplete_t( int channelID_, std::vector< BasicStream_t< typename T_::elem >* > const& streams_, StreamSettings& initSettings_ = GetDefaultChannelSettings(), OutputStamp& stamp_ = OutputStamp::GetDummyStamp() )
			{
			}
		};

#endif //#if !defined( OUTPUT_STREAM_STRIP )
	}
}

#endif // #ifndef OutputChannels_DEFINED_31_07_2019