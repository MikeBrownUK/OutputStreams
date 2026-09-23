OutputStreams 2.0

No UML accompanies this release due to time constraints. The differences to version 1.0 relate to the declaration and construction of OutputStream and OutputChannel objects. A list of major changes follows, along with a Quick Start Guide for version 2.0. I encourage you to familiarise yourself with the code for other usage, refactoring and extension possibilities (there are many).

Changes from OutputStreams 1.0

StreamsAndChannelAliases.h has been removed, therefore the type names OutputStream<> and OutputChannel<> are no more. Please use OutputStream_t<> and OutputChannel_t<> and include “OutputChannels.h” to access all library objects. Keeping the codebase small encourages my further development, nomenclature choices are often personal preference and it's very easy to add aliases in client code should you wish.
Following on from the above: when I mention OutputStreams or OutputChannels, I'm talking generally about instances of objects declared with the OutputStream_t and OutputChannel_t templates.
New guidelines for referencing stream objects globally. References should be to one of the two STREAMBASE template types that OutputStream_t and OutputChannel_t derive from: either Stream_t< CHAR_TYPE > (general use) or ConvertingStream_t< CHAR_TYPE > (for dynamic conversion of non-native strings). Such a reference can point at any OutputStream or OutputChannel declared using the same STREAMBASE.
The preprocessor define to encourage the compiler to strip away OutputStreams code and all literals sent to your streams is now “OUTPUT_STREAM_STRIP”. This seemed a sensible naming change.
GetDefaultChannelSettings() has been renamed GetDefaultStreamSettings(), reason as above.
You can either create your own buffer objects to pass to OutputStream_t and OutputChannel_t constructors or use the new OutputStreamComplete_t and OutputChannelComplete_t templates which have built-in buffers - this is the easiest option for most users.
Global scope OutputChannels in multi-thread environments should now be declared thread_local (including OutputChannelComplete_t). During this iteration I discovered that Microsoft's basic_stringbuf<> class causes an on-exit memory leak to be reported when declared thread_local. Whether this is a bug in their current implementation of basic_stringbuf<>, the compiler itself or a CRT_ALLOC... leak reporting quirk is something I can't answer at present. The leak report suggests a container or list allocation of some sort. It's just a few bytes but I don't enjoy memory leak squirt from my programs at exit, so I do hope they can fix this. It shouldn't bother library users but be aware of this buggy behaviour.

Other important notes

Please continue to use the library's streams::endl manipulator in place of std::endl. All STL manipulators I've tested are caught, forwarded and returned correctly by my code but I haven't had time to find a nice solution for std::endl with the library. In Microsoft's current header sets, std::endl is itself a templatised function and coaxing the compiler to prefer a different templatised catcher hasn't worked, so it turns the stream into a basic_ostream<> at that point (this is OK as long as you don't have further OutputStream manipulators to inject or text to convert after the first std::endl in a single chain). I will fix this issue if a solution suggests itself, but I certainly won't consider any fixes that require tweaking the 'std' namespace.

As an example of the std::endl issue, please consider:

myStream << SetDefaultPriority( 0 ); // the default

myStream << SetFilter( 4 ) << Priority( 4 ) << “All output now has priority 4 until next flush. Current stream filter permits priorities 0-4, so this text will reach output destination” << endl << Priority( 5 ) << “This text would be on a new line, following flush and with priority reset to 0, but as we have injected a new priority level, 5, this text segment will actually be discarded” << endl;

The above works correctly, outputting the first text line and rejecting the second but if you were to change the first endl (actually a mbp::streams::endl) to std::endl, the second segment would reach the output target too, because the stream reference has become a basic_ostream<> reference at that point and the Priority(5) manipulator and any other OutputStream behaviour you might expect after that just won't occur.

This 'doing nothing' behaviour for my manipulators is by design so you can quickly replace your 'myStream' (or whatever) with 'std::cout' (or whatever) directly without having to remove all OutputStream custom manipulator declarations already in code– these manipulators just silently do nothing when sent to a basic_ostream<>.

Note that the std::endl issue may change the result of any text and strings chained after it (especially with a ConvertingStream<>).

In summrary: Default, std::basic_ostream<> behaviour rules beyond the first appearance of any std::endl in a single statement – at least for now.

Quick Start Guide:

All examples assume

#include “OutputChannels.h”

To define an OutputStream of char type called gMyStream with encapsulated buffer, writing to standard output (std::cout) with no message prefixing:

using namespace mbp::streams;

OutputStreamComplete_t< OutputStdOut_t< char >, Stream_t< char > > gMyStream( GetDefaultChannelSettings(), OutputStamp::GetDummyStamp() );

The above constructs a stream object using default stream settings. Everything will be passed to output until/if you change filter and priority options and no prefixes will be added to your output. Some of the construction parameters are defaults and can be ommited, but they are given explicitly above so it's obvious where to change to add different message prefixes or pass in different initial settings.

Usage:

Stream_t< char >& gOut = gMyStream;

gMyStream << “The value of x in hexadecimal is: “ << std::hex << x << endl;

gOut << “This goes to the same stream” << endl;

Referencing for other modules (within a header):

#include “OutputChannels.h”

extern mbp::streams::Stream_t< char >& gOut;

Other module that includes the header:

using namespace mbp::streams;

gOut << Priority(INFO) << “going to main stream again providing filter is currently (INFO) or lower” << endl;

INFO above is an integer alias, perhaps a define or enum – that is up to client to define. 0 = highest Priority in OutputStreams, therefore a manipulator or function call to 'Filter(2)' will stop any message with 'Priority(3)' or greater from reaching the final OutputTarget (which here is std::cout). Priority(x) calls last until the buffer is next flushed via flush/endl manipulators or explicit function call, at which time Priority is reset to that stream or channel's current DefaultPriority (which is 0 at construction with GetDefaultStreamSettings() and changeable itself via a manipulator or function call).

Changing gOut to be a channel for multi-threaded use is easy enough:

thread_local OutputChannelComplete_t< Stream_t< char >, true > gMyChannel( 0, { &gMyStream }, GetDefaultChannelSettings(), SystemTimeStamp_t< char >::GetInstance() );

The above creates a channel with channel ID of 0 (again, the intent is that you replace numbers with your own enums here) that connects to the gMyStream object and shares settings with all other channel objects using that ID (across all threads). The object created above will again use default stream settings, but adds a timestamp prefix at each flush of the channel (remember that the message prefixing classes are easily extensible – all working examples are in OutputStamp.h).

Note that the second, boolean, template parameter to OutputChannelComplete_t<> is the multi-thread flag. Specify false for slightly optimised throughput in a single thread environment if you still need a channel (perhaps many channels to one stream or many channels to many streams) .

For my own diagnostics / general output whilst developing, I tend to use one OutputStream with one thread_local OutputChannel that connects to it, very much like the above examples, with a reference to it in my main header, thus:

extern thread_local Stream_t< char >& gDbgOut;

That reference is connected up to the thread_local channel at point of definition much like the stream example. Streams and channels both derive (statically) from STREAMBASE< CHAR_TYPE > - in this case Stream_t< char > so the reference can be pointed at either object.

Finally...

A reminder that OutputStreams is licenced under the MIT license, a copy of which is included with the source distribution.

I hope you find the library useful.

﻿
