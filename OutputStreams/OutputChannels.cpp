//////////////////////////////////////////////////////////////////////////
/// Mike Brown, 2022
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
		uint8_t g_channelInitFlags[ kMaxOutputChannels ]{};
		uint8_t g_streamInitFlags[ kPrime ]{};
		void * g_allSharedStreams[ kPrime ]{};
		
		std::mutex g_streamsMutex;

		size_t GetIndexFromPointer( void * ptr_ )
		{
			size_t index = reinterpret_cast< ptrdiff_t >( ptr_ ) % kPrime;
			bool found = false;
			while ( !found )
			{
				if ( g_allSharedStreams[ index ] == nullptr || g_allSharedStreams[ index ] == ptr_ )
					return index;
				index = ++index % kPrime;
			}
			return index;
		}
	}
}
