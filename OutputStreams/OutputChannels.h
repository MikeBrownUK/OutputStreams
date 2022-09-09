//////////////////////////////////////////////////////////////////////////
/// Mike Brown, 2019
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
		extern uint8_t g_channelInitFlags[ kMaxOutputChannels ];
		// array of shared stream pointers and initialisation flags used to prevent new channels from performing reinitialisation when attaching to the same one
		extern void * g_allSharedStreams[ kPrime ];
		extern uint8_t g_streamInitFlags[ kPrime ];
		// mutex protecting the above
		extern std::mutex g_streamsMutex;
		// hash and probe function
		size_t GetIndexFromPointer( void * ptr_ );

		template < typename ELEM_, bool MULTITHREAD_ >
		class ChannelBuffer_t;

		// An OutputChannel uses a Channel ID and attaches to one or more OutputStreams, allowing per-channel filtering of output
		template< typename STREAMBASE_ >
		class OutputChannel_t : public STREAMBASE_
		{
			using base = STREAMBASE_;
			using type = typename STREAMBASE_::type;
		public:
			OutputChannel_t( int channelID_, std::vector< BasicStream_t< type > * > const& streams_, bool isMultiThreadChannel_ = true,  OutputStamp & stamp_ = OutputStamp::GetDummyStamp(), StreamSettings * initSettings_ = &GetDefaultChannelSettings() )
				: base( nullptr, initSettings_, stamp_ )
				, m_channelId( channelID_ )
				, m_sharedStreams( streams_ )
			{
				assert( channelID_ < kMaxOutputChannels );
				// create the correct ChannelBuffer for single or multi-thread usage
				if ( isMultiThreadChannel_ )
					BasicStream_t< type >::rdbuf( new ChannelBuffer_t< STREAMBASE_, true >( *this, m_sharedStreams ) );
				else
					BasicStream_t< type >::rdbuf( new ChannelBuffer_t< STREAMBASE_, false >( *this, m_sharedStreams ) );

				g_streamsMutex.lock();
				if ( 0 == g_channelInitFlags[ channelID_ ]++ )
				{
					g_AllChannelSettings[ channelID_ ].Enable( initSettings_->GetEnable() );
					g_AllChannelSettings[ channelID_ ].SetPriority( initSettings_->GetPriority() );
					g_AllChannelSettings[ channelID_ ].SetDefaultPriority( initSettings_->GetDefaultPriority() );
					g_AllChannelSettings[ channelID_ ].SetFilter( initSettings_->GetFilter() );
				}
				for ( auto *& i : m_sharedStreams )
				{
					size_t index = GetIndexFromPointer( i );
					g_allSharedStreams[ index ] = i;
					if ( 0 == g_streamInitFlags[ index ]++ )
					{
						i->SetIsChannelTarget( true );
						BasicBuffer_t< type > * buff = static_cast< BasicBuffer_t< type > * >( i->rdbuf() );
						buff->SetOriginalBufferStart();
					}
				}
				g_streamsMutex.unlock();
			}
			virtual ~OutputChannel_t()
			{
				size_t index;
				delete base::rdbuf();
				{
					g_streamsMutex.lock();
					--g_channelInitFlags[ m_channelId ];
					for ( auto *& i : m_sharedStreams )
					{
						index = GetIndexFromPointer( i );
						if ( 1 == g_streamInitFlags[ index ]-- )
						{
							g_allSharedStreams[ index ] = nullptr;
							i->SetIsChannelTarget( false );
							BasicBuffer_t< type > * buff = static_cast< BasicBuffer_t< type > * >( i->rdbuf() );
							buff->ReserveStamp( *i );
						}
					}
					g_streamsMutex.unlock();
				}
			}
			// enable channel and filter functions for the shared stream (all threads)
			virtual void Enable( SettingsType enable_ ) override { g_AllChannelSettings[ m_channelId ].Enable( enable_ ); }
			virtual SettingsType GetEnable() override { return g_AllChannelSettings[ m_channelId ].GetEnable(); }
			virtual void SetPriority( SettingsType newPriority_ ) override { g_AllChannelSettings[ m_channelId ].SetPriority( newPriority_ ); }
			virtual SettingsType GetPriority() override { return g_AllChannelSettings[ m_channelId ].GetPriority(); }
			virtual void SetDefaultPriority( SettingsType newDefault_ ) override {
				g_AllChannelSettings[ m_channelId ].SetDefaultPriority( newDefault_ ); g_AllChannelSettings[ m_channelId ].SetPriority( newDefault_ );
			}
			virtual SettingsType GetDefaultPriority() override { return g_AllChannelSettings[ m_channelId ].GetDefaultPriority(); }
			virtual void SetFilter( SettingsType newFilter_ ) override { g_AllChannelSettings[ m_channelId ].SetFilter( newFilter_ ); }
			virtual SettingsType GetFilter() override { return g_AllChannelSettings[ m_channelId ].GetFilter(); }
			int const GetChannelId() const { return m_channelId; }
		private:
			int const m_channelId;
			std::vector < BasicStream_t< type > * > m_sharedStreams;
			OutputChannel_t() = delete;
			OutputChannel_t( OutputChannel_t const & other_ ) = delete;
			OutputChannel_t operator=( OutputChannel_t const & other_ ) = delete;
		};

		// ChannelBuffer is an OutputChannel's buffer specialisation
		template < typename STREAMBASE_, bool MULTITHREAD_ = true >
		class ChannelBuffer_t : public std::basic_stringbuf< typename STREAMBASE_::type, std::char_traits< typename STREAMBASE_::type >, std::allocator< typename STREAMBASE_::type > >
		{
		protected:
			using type = typename STREAMBASE_::type;
			using traits = std::char_traits < type >;
			using base = std::basic_stringbuf< type, traits, std::allocator< type > >;
		public:
			ChannelBuffer_t( OutputChannel_t< STREAMBASE_ > & local_, std::vector< BasicStream_t< type > * > & shared_ )
				: m_localChannel( local_ )
			{
				for( auto *&i : shared_ )
				{
					m_streamIndices.push_back( GetIndexFromPointer( i ) );
				}
				auto len = local_.GetOutputStamp().GetMaxLength();
				for ( auto i = 0; i < len; ++i )
				{
					base::sputc( 'C' );
				}
			}
			virtual ~ChannelBuffer_t()	{}
		protected:
			virtual int sync() override
			{
				BasicStream_t< type > * strm_;
				int maxLength = 0;
				int stampLength = 0;	
				uint8_t writesComplete[ kMaxSharedStreams ];
				auto numCharacters = base::pptr() - base::pbase();
				if ( g_AllChannelSettings[ m_localChannel.GetChannelId() ].CanBeOutput() )
				{
					maxLength = m_localChannel.GetOutputStamp().GetMaxLength();
					m_localChannel.GetOutputStamp().Lock();
					stampLength = m_localChannel.GetOutputStamp().GetLength();
					auto offset = maxLength - stampLength;
					m_localChannel.GetOutputStamp().WriteStamp( base::pbase() + offset );
					m_localChannel.GetOutputStamp().Unlock();
					numCharacters -= maxLength;
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
								strm_ = reinterpret_cast< BasicStream_t < type > * >( g_allSharedStreams[ i ] );
								if ( strm_ && strm_->m_settings.CanBeOutput() )
								{
									if ( strm_->TryLock() )
									{
										strm_->write( base::pbase() + offset, numCharacters + stampLength );
										strm_->flush();
										strm_->Unlock();
										++writesComplete[ j ];
									}
								}
								else
									++writesComplete[ j ];
								done &= writesComplete[ j++ ];
							}
						}
					} while ( !done );
				}
				base::pbump( -static_cast< int >( numCharacters ) );
				g_AllChannelSettings[ m_localChannel.GetChannelId() ].SetPriority( g_AllChannelSettings[ m_localChannel.GetChannelId() ].GetDefaultPriority() );
				return 0;
			}
			OutputChannel_t< STREAMBASE_ > & m_localChannel;
			std::vector< size_t > m_streamIndices;
			ChannelBuffer_t() = delete;
			ChannelBuffer_t( ChannelBuffer_t const & rhs_ ) = delete;
			ChannelBuffer_t & operator = ( ChannelBuffer_t const & rhs_ ) = delete;
		};

		// ChannelBuffer specialisation for single-thread use
		template< typename STREAMBASE_ >
		class ChannelBuffer_t< STREAMBASE_, false > : public ChannelBuffer_t< STREAMBASE_, true  >
		{
		public:
			using base = ChannelBuffer_t< STREAMBASE_, true >;
			using type = typename base::type;

			ChannelBuffer_t( OutputChannel_t< STREAMBASE_ > & local_, std::vector< BasicStream_t< type > * > & shared_ )
				: ChannelBuffer_t< STREAMBASE_, true >( local_, shared_ )
			{
			}
			virtual ~ChannelBuffer_t() {}
			virtual int sync() override
			{
				BasicStream_t< type > * strm_;

				auto maxLength = base::m_localChannel.GetOutputStamp().GetMaxLength();
				auto stampLength = base::m_localChannel.GetOutputStamp().GetLength();
				auto offset = maxLength - stampLength;
				auto numCharacters = base::pptr() - base::pbase() - maxLength;

				if ( g_AllChannelSettings[ base::m_localChannel.GetChannelId() ].CanBeOutput() )
				{
					base::m_localChannel.GetOutputStamp().WriteStamp( base::pbase() + offset );
					for ( auto i : base::m_streamIndices )
					{
						strm_ = reinterpret_cast< BasicStream_t < type > * >( g_allSharedStreams[ i ] );
						if ( strm_->m_settings.CanBeOutput() )
						{
							strm_->write( base::pbase() + offset, numCharacters + stampLength );
							strm_->flush();
						}
					}
				}
				base::pbump( -static_cast< int >( numCharacters ) );
				g_AllChannelSettings[ base::m_localChannel.GetChannelId() ].SetPriority( g_AllChannelSettings[ base::m_localChannel.GetChannelId() ].GetDefaultPriority() );
				return 0;
			}
		};
	}
}

#endif // #ifndef OutputChannels_DEFINED_31_07_2019