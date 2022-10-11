//////////////////////////////////////////////////////////////////////////
/// Mike Brown, 2022
///
///	Filename: 	OutputStamp.h
///	Created:	21/06/2022
///	Author:		Mike Brown
///	
///	Description: A class to write an optional prefix stamp to an OutputStream and OutputChannel
///				 Base class OutputStamp writes nothing
/// 
///				 You must provide a virtual GetLength function that returns the length of the next TimeStamp in characters (not bytes). 
//////////////////////////////////////////////////////////////////////////

#ifndef OutputStamp_DEFINED_21_06_2022
#define OutputStamp_DEFINED_21_06_2022

#include <chrono>
#include <mutex>

namespace mbp
{
	namespace streams
	{
		class OutputStamp
		{
		public:
			virtual int GetMaxLength() const { return 0; }
			virtual int GetLength() { return 0; };
			virtual int WriteStamp( void * ptr_ = nullptr ) { return 0; }
			virtual void Lock() {}
			virtual void Unlock() {}
			static OutputStamp & GetDummyStamp()
			{
				static OutputStamp instance;
				return instance;
			}
			OutputStamp() = default;
			virtual ~OutputStamp() = default;
		};

		// an example Timestamp using the system clock to write fixed length date and time
		template< typename T_ >
		class SystemTimeStamp_t : public OutputStamp
		{
		public:
			static SystemTimeStamp_t & GetInstance() { static SystemTimeStamp_t inst; return inst; }
			virtual int GetMaxLength() const { return m_kNumberOfCharacters; }
			virtual int GetLength() { return m_kNumberOfCharacters; }
			virtual int WriteStamp( void * ptr_ = nullptr );
			virtual ~SystemTimeStamp_t() = default;
		private:
			SystemTimeStamp_t()
			{
				// we are fixed length so we can generate one time string and cache the number of characters
				m_kNumberOfCharacters = static_cast< uint32_t >( WriteStamp( nullptr ) );
			}
			uint32_t m_kNumberOfCharacters;
		};

		// an example of OutputStamping with varying length - a simple 'line number' prefixer
		template< typename T_ >
		class LineStamp_t : public OutputStamp
		{
		public:
			static LineStamp_t & GetInstance() { static LineStamp_t inst; return inst; }
			virtual int GetMaxLength() const { return 32; }		// purely arbitrary (and somewhat ridiculous for a line count)
			virtual int GetLength();
			virtual int WriteStamp( void * ptr_ = nullptr );
			virtual ~LineStamp_t() = default;
			virtual void Lock() { m_mutex.lock(); }
			virtual void Unlock() { m_mutex.unlock(); }
		private:
			LineStamp_t()
				: m_counter( 0 )
			{}
			uint32_t m_counter;
			uint32_t m_kNumberOfCharacters;
			std::mutex m_mutex;
		};

		template< typename T_ >
		int mbp::streams::LineStamp_t<T_>::GetLength()
		{
			std::basic_stringstream< T_ > strm;
			strm << m_counter++ << " ";
			return strm.str().length();
		}

		template< typename T_ >
		int mbp::streams::LineStamp_t<T_>::WriteStamp( void * ptr_ /* = nullptr */  )
		{
			std::basic_stringstream< T_ > strm;
			strm << m_counter << " ";
			auto lenStr = strm.str().length();
			if( ptr_ )
				memcpy( ptr_, strm.str().c_str(), ( lenStr ) * sizeof( T_ ) );
			return lenStr;
		}
	}
}

#endif // #ifndef OutputStamp_DEFINED_21_06_2022