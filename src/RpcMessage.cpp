/**
 *          Copyright Springbeats Sarl 2013.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file ../LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
#include "RpcMessage.hpp"
#include "RpcController.hpp"
#include "Util.hpp"
#include <sstream>

namespace pbrpcpp {    
    void RpcMessage::serializeRequest( const string& callId,
                const MethodDescriptor& method,
                const Message& request,
                ostream& out ) {
        Util::writeInt( REQUEST_MSG, out );
        Util::writeString( callId, out );
        Util::writeMethodDescriptor( method, out );
        Util::writeMessage( request, out );
    }

    void RpcMessage::serializeResponse( const string& callId, 
                                        const RpcController& controller,    
                                        const Message* response,
                                        ostream& out ) {

        Util::writeInt( RESPONSE_MSG, out );
        Util::writeString( callId, out );
        Util::writeController( controller, out );
        if( response ) {
            out.put( 'Y');
            Util::writeMessage( *response, out );
        } else {
            out.put( 'N');
        }
    }


    void RpcMessage::serializeCancel( const string& callId, ostream& out ) {
        Util::writeInt( CANCEL_MSG, out );
        Util::writeString( callId, out );
    }


    void RpcMessage::parseRequestFrom( istream& in, string& callId, const MethodDescriptor*& method, shared_ptr<Message>& request ) {
        callId = Util::readString( in );
        method = Util::readMethodDescriptor( in );
        request.reset();
        Util::readMessage( in, request );
    }

    void RpcMessage::parseResponseFrom( istream& in, string& callId, RpcController& controller, shared_ptr<Message>& response ) {
        callId = Util::readString( in );
        Util::readController( in, controller );
        char c = in.get();
        if( c == 'Y') {
          response.reset();
          Util::readMessage( in, response );
        }
    }

    void RpcMessage::parseCancelFrom( istream& in, string& callId ) {
        callId = Util::readString( in );
    }
    
    string RpcMessage::serializeNetPacket( const string& packet ) {
        std::ostringstream out;
        
        //write packet start flag: 1 byte
        Util::writeChar( 'R', out );
        
        //write the content length ( 4 bytes ) and content itself
        Util::writeString( packet, out);
        
        return out.str();
    }
    
    bool RpcMessage::extractNetPacket( string& s, string& packet ) {
        size_t pos = 0;

        //at least 5 bytes for a message
        if( s.length() < 5 ) {
            return false;
        }

        //read start flag
        char ch = Util::readChar(s, pos);
        //read content length
        int n = Util::readInt(s, pos);
        
        if( ch != 'R' || n < 0 ) {
            throw runtime_error( "invalid network packet");
        }
        
        //if s
        if( pos + n > s.length() ) {
            return false;
        }
        
        packet = s.substr( pos, n );        
        s.erase(0, n + pos);
        
        return true;
    }

}//end name space pbrpcpp



