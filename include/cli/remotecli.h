/*******************************************************************************
 * CLI - A simple command line interface.
 * Copyright (C) 2016 Daniele Pallastrelli
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************/

#ifndef CLI_REMOTECLI_H_
#define CLI_REMOTECLI_H_

#include <cli/inputhandler.h>
#include <memory>
#include "cli.h"
#include "server.h"
#include "inputdevice.h"
#include "boostasio.h"

namespace cli
{

// *******************************************************************************

class TelnetSession : public Session
{
public:
    TelnetSession(boost::asio::ip::tcp::socket socket) :
        Session(std::move(socket))
    {}

protected:

    virtual std::string Encode(const std::string& data) const override
    {
        std::string result;
        for (char c: data)
        {
            if (c == '\n') result += '\r';
            result += c;
        }
        return result;
    }

    virtual void OnConnect() override
    {
        // to specify hexadecimal value as chars we use
        // the syntax \xVVV
        // and the std::string ctor that takes the size,
        // so that it's not null-terminated
        //std::string msg{ "\x0FF\x0FD\x027", 3 };
        //waitAck = true;
        //std::string iacDoSuppressGoAhead{ "\x0FF\x0FD\x003", 3 };
        //this -> OutStream() << iacDoSuppressGoAhead << std::flush;

        // https://www.ibm.com/support/knowledgecenter/SSLTBW_1.13.0/com.ibm.zos.r13.hald001/telcmds.htm

        std::string iacDoLineMode{ "\x0FF\x0FD\x022", 3 };
        this -> OutStream() << iacDoLineMode << std::flush;

        std::string iacSbLineMode0IacSe{ "\x0FF\x0FA\x022\x001\x000\x0FF\x0F0", 7 };
        this -> OutStream() << iacSbLineMode0IacSe << std::flush;

        std::string iacWillEcho{ "\x0FF\x0FB\x001", 3 };
        this -> OutStream() << iacWillEcho << std::flush;
/*
        constexpr char IAC = '\x0FF'; // 255
        constexpr char DO = '\x0FD'; // 253
        constexpr char VT100 = '\x030'; // 48

        this -> OutStream() << IAC << DO << VT100 << std::flush;
*/
        //cliSession.Prompt();
    }
    virtual void OnDisconnect() override {}
    virtual void OnError() override {}
#if 0
    virtual void OnDataReceived(const std::string& data) override
    {
        if (waitAck)
        {
            if ( data[0] == '\x0FF' )
            {
                // TODO
                for (size_t i = 0; i < data.size(); ++i)
                    std::cout << static_cast<int>( data[i] & 0xFF ) << ' ';
                std::cout << std::endl;

                for (size_t i = 0; i < data.size(); ++i)
                    std::cout << "0x" << std::hex << static_cast<int>( data[i] & 0xFF ) << std::dec << ' ';
                std::cout << std::endl;

                for (size_t i = 0; i < data.size(); ++i)
                    switch (static_cast<int>( data[i] & 0xFF ))
                    {
                        case 0xFF: std::cout << "IAC "; break;
                        case 0xFE: std::cout << "DONT "; break;
                        case 0xFD: std::cout << "DO "; break;
                        case 0xFC: std::cout << "WONT "; break;
                        case 0xFB: std::cout << "WILL "; break;
                        case 0xFA: std::cout << "SB "; break;
                        case 0xF9: std::cout << "GoAhead "; break;
                        case 0xF8: std::cout << "EraseLine "; break;
                        case 0xF7: std::cout << "EraseCharacter "; break;
                        case 0xF6: std::cout << "AreYouThere "; break;
                        case 0xF5: std::cout << "AbortOutput "; break;
                        case 0xF4: std::cout << "InterruptProcess "; break;
                        case 0xF3: std::cout << "Break "; break;
                        case 0xF2: std::cout << "DataMark "; break;
                        case 0xF1: std::cout << "NOP "; break;
                        case 0xF0: std::cout << "SE "; break;
                        default: std::cout << (static_cast<int>( data[i] & 0xFF )) << ' ';
                    }
                std::cout << std::endl;

            }
            waitAck = false;
/*
            std::string iacWillSuppressGoAhead{ "\x0FF\x0FB\x003", 3 };
            if ( data == iacWillSuppressGoAhead )
            {
                waitAck = false;
                cliSession.Prompt();
            }

            else
                Disconnect();
*/
        }
        else
        {
            for (size_t i = 0; i < data.size(); ++i)
                Feed(data[i]);
/*
            auto str = data;
            // trim trailing spaces
            std::size_t endpos = str.find_last_not_of(" \t\r\n");
            if( std::string::npos != endpos ) str = str.substr( 0, endpos+1 );

            if ( cliSession.Feed( str ) ) cliSession.Prompt();
            else Disconnect();
*/
        }
    }
#else

    enum
    {
        SE = '\x0F0',                  // End of subnegotiation parameters.
        NOP = '\x0F1',                 // No operation.
        DataMark = '\x0F2',            // The data stream portion of a Synch.
                                       // This should always be accompanied
                                       // by a TCP Urgent notification.
        Break = '\x0F3',               // NVT character BRK.
        InterruptProcess = '\x0F4',    // The function IP.
        AbortOutput = '\x0F5',         // The function AO.
        AreYouThere = '\x0F6',         // The function AYT.
        EraseCharacter = '\x0F7',      // The function EC.
        EraseLine = '\x0F8',           // The function EL.
        GoAhead = '\x0F9',             // The GA signal.
        SB = '\x0FA',                  // Indicates that what follows is
                                       // subnegotiation of the indicated
                                       // option.
        WILL = '\x0FB',                // Indicates the desire to begin
                                       // performing, or confirmation that
                                       // you are now performing, the
                                       // indicated option.
        WONT = '\x0FC',                // Indicates the refusal to perform,
                                       // or continue performing, the
                                       // indicated option.
        DO = '\x0FD',                  // Indicates the request that the
                                       // other party perform, or
                                       // confirmation that you are expecting
                                       // the other party to perform, the
                                       // indicated option.
        DONT = '\x0FE',                // Indicates the demand that the
                                       // other party stop performing,
                                       // or confirmation that you are no
                                       // longer expecting the other party
                                       // to perform, the indicated option.
        IAC = '\x0FF'                  // Data Byte 255.
    };


    virtual void OnDataReceived(const std::string& data) override
    {
        for (char c: data)
            Consume(c);
    }

private:

    void Consume(char c)
    {
        if (escape)
        {
            if (c == IAC)
            {
                Data(c);
                escape = false;
            }
            else
            {
                Command(c);
                escape = false;
            }
        }
        else
        {
            if (c == IAC)
                escape = true;
            else
                Data(c);
        }
    }

    void Data(char c)
    {
        switch(state)
        {
            case State::data:
                Output(c);
                break;
            case State::sub:
                Sub(c);
                break;
            case State::wait_will:
                Will(c);
                state = State::data;
                break;
            case State::wait_wont:
                Wont(c);
                state = State::data;
                break;
            case State::wait_do:
                Do(c);
                state = State::data;
                break;
            case State::wait_dont:
                Dont(c);
                state = State::data;
                break;
        }
    }

    void Command(char c)
    {
/*
        switch (static_cast<int>( c & 0xFF ))
        {
            case 0xFF: std::cout << "IAC" << std::endl; break;
            case 0xFE: std::cout << "DONT" << std::endl; break;
            case 0xFD: std::cout << "DO" << std::endl; break;
            case 0xFC: std::cout << "WONT" << std::endl; break;
            case 0xFB: std::cout << "WILL" << std::endl; break;
            case 0xFA: std::cout << "SB" << std::endl; break;
            case 0xF9: std::cout << "GoAhead" << std::endl; break;
            case 0xF8: std::cout << "EraseLine" << std::endl; break;
            case 0xF7: std::cout << "EraseCharacter" << std::endl; break;
            case 0xF6: std::cout << "AreYouThere" << std::endl; break;
            case 0xF5: std::cout << "AbortOutput" << std::endl; break;
            case 0xF4: std::cout << "InterruptProcess" << std::endl; break;
            case 0xF3: std::cout << "Break" << std::endl; break;
            case 0xF2: std::cout << "DataMark" << std::endl; break;
            case 0xF1: std::cout << "NOP" << std::endl; break;
            case 0xF0: std::cout << "SE" << std::endl; break;
            default: std::cout << (static_cast<int>( c & 0xFF )) << std::endl;;
        }
*/
        switch(c)
        {
            case SE:
                if (state == State::sub)
                    state = State::data;
                else
                    std::cout << "ERROR: received SE when not in sub state" << std::endl;
                break;
            case DataMark: // ?
            case Break: // ?
            case InterruptProcess:
            case AbortOutput:
            case AreYouThere:
            case EraseCharacter:
            case EraseLine:
            case GoAhead:
            case NOP:
                state = State::data;
                break;
            case SB:
                if (state != State::sub)
                    state = State::sub;
                else
                    std::cout << "ERROR: received SB when already in sub state" << std::endl;
                break;
            case WILL:
                state = State::wait_will;
                break;
            case WONT:
                state = State::wait_wont;
                break;
            case DO:
                state = State::wait_do;
                break;
            case DONT:
                state = State::wait_dont;
                break;
            case IAC:
                assert(false); // can't be here
                state = State::data;
                break;
        }
    }

    void Will(char c) 
    { 
        #ifdef CLI_TELNET_TRACE
        std::cout << "will " << static_cast<int>(c) << std::endl;
        #else
        (void)c;
        #endif
    }
    void Wont(char c)
    { 
        #ifdef CLI_TELNET_TRACE
        std::cout << "wont " << static_cast<int>(c) << std::endl;
        #else
        (void)c;
        #endif
    }
    void Do(char c)
    { 
        #ifdef CLI_TELNET_TRACE
        std::cout << "do " << static_cast<int>(c) << std::endl;
        #else
        (void)c;
        #endif
    }
    void Dont(char c) 
    {
        #ifdef CLI_TELNET_TRACE
        std::cout << "dont " << static_cast<int>(c) << std::endl;
        #else
        (void)c;
        #endif
    }
    void Sub(char c) 
    { 
        #ifdef CLI_TELNET_TRACE
        std::cout << "sub: " << static_cast<int>(c) << std::endl;
        #else
        (void)c;
        #endif
    }
protected:
    virtual void Output(char c)
    {
        #ifdef CLI_TELNET_TRACE
        std::cout << "data: " << static_cast<int>(c) << std::endl;
        #else
        (void)c;
        #endif
    }
private:
    enum class State { data, sub, wait_will, wait_wont, wait_do, wait_dont };
    State state = State::data;
    bool escape = false;

#endif

private:
    void Feed(char c)
    {
        if (std::isprint(c)) std::cout << c << std::endl;
        else std::cout << "0x" << std::hex << static_cast<int>(c) << std::dec << std::endl;
/*
        switch ( c )
        {
        case 0: break;
        case '\n':
        case '\r':
        {
            // trim trailing spaces
            std::size_t endpos = buffer.find_last_not_of(" \t\r\n");
            if( std::string::npos != endpos ) buffer = buffer.substr( 0, endpos+1 );
            if ( cliSession.Feed( buffer ) ) cliSession.Prompt();
            else Disconnect();

            buffer.clear();
            break;
        }
        default:
            Echo( c );
            buffer += c;
        }
*/
    }
/*
    void Echo( char c )
    {
        this -> OutStream() << c << std::flush;
    }
*/
    std::string buffer;
    //bool waitAck = false;
};

class TelnetServer : public Server
{
public:
    TelnetServer(detail::asio::BoostExecutor::ContextType& ios, short port) :
        Server(ios, port)
    {}
    virtual std::shared_ptr< Session > CreateSession(boost::asio::ip::tcp::socket socket) override
    {
        return std::make_shared<TelnetSession>(std::move(socket));
    }
};

//////////////

class CliTelnetSession : public InputDevice, public TelnetSession, public CliSession
{
public:

    CliTelnetSession(boost::asio::ip::tcp::socket socket, Cli& cli, std::function< void(std::ostream&)> exitAction, std::size_t historySize ) :
        InputDevice(detail::asio::BoostExecutor(socket)),
        TelnetSession(std::move(socket)),
        CliSession(cli, TelnetSession::OutStream(), historySize),
        poll(*this, *this)
    {
        ExitAction([this, exitAction](std::ostream& out){ exitAction(out), Disconnect(); } );
    }
protected:

    virtual void OnConnect() override
    {
        TelnetSession::OnConnect();
        Prompt();
    }

    void Output(char c) override
    {
        switch(step)
        {
            case Step::_1:
                switch( c )
                {
                    case 127: Notify(std::make_pair(KeyType::backspace,' ')); break;
                    //case 10: Notify(std::make_pair(KeyType::ret,' ')); break;
                    case 27: step = Step::_2; break;  // symbol
                    case 13: step = Step::wait_0; break;  // wait for 0 (ENTER key)
                    default: // ascii
                    {
                        const char ch = static_cast<char>(c);
                        Notify(std::make_pair(KeyType::ascii,ch));
                    }
                }
                break;

            case Step::_2: // got 27 last time
                if ( c == 91 )
                {
                    step = Step::_3;
                    break;  // arrow keys
                }
                else
                {
                    step = Step::_1;
                    Notify(std::make_pair(KeyType::ignored,' '));
                    break; // unknown
                }
                break;

            case Step::_3: // got 27 and 91
                switch( c )
                {
                    case 51: step = Step::_4; break;  // not arrow keys
                    case 65: step = Step::_1; Notify(std::make_pair(KeyType::up,' ')); break;
                    case 66: step = Step::_1; Notify(std::make_pair(KeyType::down,' ')); break;
                    case 68: step = Step::_1; Notify(std::make_pair(KeyType::left,' ')); break;
                    case 67: step = Step::_1; Notify(std::make_pair(KeyType::right,' ')); break;
                    case 70: step = Step::_1; Notify(std::make_pair(KeyType::end,' ')); break;
                    case 72: step = Step::_1; Notify(std::make_pair(KeyType::home,' ')); break;
                }
                break;

            case Step::_4:
                if ( c == 126 ) Notify(std::make_pair(KeyType::canc,' '));
                else Notify(std::make_pair(KeyType::ignored,' '));

                step = Step::_1;

                break;

            case Step::wait_0:
                if ( c == 0 /* linux */ || c == 10 /* win */ ) Notify(std::make_pair(KeyType::ret,' '));
                else Notify(std::make_pair(KeyType::ignored,' '));

                step = Step::_1;

                break;

        }
    }

private:

    enum class Step { _1, _2, _3, _4, wait_0 };
    Step step = Step::_1;
    InputHandler poll;
};


class CliTelnetServer : public Server
{
public:
    CliTelnetServer(detail::asio::BoostExecutor::ContextType& ios, short port, Cli& _cli, std::size_t _historySize=100 ) :
        Server(ios, port),
        cli(_cli),
        historySize(_historySize)
    {}
    CliTelnetServer(detail::asio::BoostExecutor::ContextType& ios, std::string address, short port, Cli& _cli, std::size_t _historySize=100 ) :
        Server(ios, address, port),
        cli(_cli),
        historySize(_historySize)
    {}
    void ExitAction( std::function< void(std::ostream&)> action )
    {
        exitAction = action;
    }
    virtual std::shared_ptr<Session> CreateSession(boost::asio::ip::tcp::socket socket) override
    {
        return std::make_shared<CliTelnetSession>(std::move(socket), cli, exitAction, historySize);
    }
private:
    Cli& cli;
    std::function< void(std::ostream&)> exitAction;
    std::size_t historySize;
};


} // namespace cli

#endif // CLI_REMOTECLI_H_
