//////////////////////////////////////////////////////////////////////////
/// ©Mike Brown, 2014-2026
/// https://www.mikebrown.co.uk
/// 
/// Filename:	OutputChannels.cpp
/// Created:	18/7/2022
/// Author:		Mike
/// 
/// Description:
///
//////////////////////////////////////////////////////////////////////////

#include "OutputChannels.h"

namespace mbp
{
	namespace streams
	{
		StreamSettings g_AllChannelSettings[ kMaxOutputChannels ]{};

		std::uint8_t * GetStreamInitFlags()
		{
			static uint8_t g_streamInitFlags[ kPrime ]{};
			return g_streamInitFlags;
		}

		// returns whether a channel ID has been initialised
		std::uint8_t * GetChannelInitFlags()
		{
			static uint8_t g_channelInitFlags[ kMaxOutputChannels ]{};
			return g_channelInitFlags;
		}

		void ** GetSharedStreamArray()
		{
			static void * g_allSharedStreams[ kPrime ] = {};
			return g_allSharedStreams;
		}

		std::mutex & GetMasterChannelMutex()
		{
			static std::mutex g_masterMutex;
			return g_masterMutex;
		}

		size_t GetIndexFromPointer( void * ptr_ )
		{
			size_t index = reinterpret_cast< ptrdiff_t >( ptr_ ) % kPrime;
			bool found = false;
			while ( !found )
			{
				if ( GetSharedStreamArray()[ index ] == nullptr || GetSharedStreamArray()[ index ] == ptr_ )
					return index;
				index = ++index % kPrime;
			}
			return index;
		}
	}
}
