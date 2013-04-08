/* 
 * File:   BaseRpcServer.hpp
 * Author: Steven
 *
 * Created on March 31, 2013, 7:21 PM
 */

#ifndef BASERPCSERVER_HPP
#define	BASERPCSERVER_HPP


#include <string>
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>
#include <google/protobuf/service.h>
#include "RpcController.hpp"
#include "Queue.hpp"
#include "ThreadSafeMap.hpp"

using boost::shared_ptr;
using std::string;
using google::protobuf::Service;
using google::protobuf::ServiceDescriptor;
using google::protobuf::Message;
using google::protobuf::MethodDescriptor;

namespace pbrpcpp {
    
    class BaseRpcServer: public boost::noncopyable {
    public:
        virtual ~BaseRpcServer();
        void Export( Service* service );        
    protected:
        BaseRpcServer();        
        void messageReceived( int clientId, const string& msg );
        virtual void sendResponse( int clientId, const string& msg ) = 0;
    protected:
        volatile bool stop_;
    private:
        struct ClientMsg {
            ClientMsg( int _clientId, const string& _msg )
            :clientId( _clientId ),
            msg( _msg )
            {                
            }
            
            int clientId;
            string msg;
        };
        
        struct ClientCallId {
            
            ClientCallId( int _clientId, const string& _callId )
            :clientId( _clientId ),
            callId( _callId )
            {
                
            }
            bool operator==( const ClientCallId& right ) const {
                return this->clientId == right.clientId && this->callId == right.callId;
            }
            
            bool operator<( const ClientCallId& right ) const {
                return this->clientId < right.clientId || ( this->clientId == right.clientId && this->callId < right.callId );
            }
            int clientId;
            string callId;
                    
        };
        struct Request {
            Request( int _clientId, const string& _callId, RpcController* _controller, Message* _requestMsg, Message* _responseMsg )
            :clientId( _clientId ),
            callId( _callId ),
            controller( _controller ),
            requestMsg( _requestMsg ),
            responseMsg( _responseMsg )
            {
            }

            ~Request() {
                delete controller;
                delete requestMsg;
                delete responseMsg;
            }
            int clientId;
            string callId;
            RpcController* controller;
            Message* requestMsg;
            Message* responseMsg;
        };
    private:
        void takeAndProcessMsg( Queue< ClientMsg* >& msgQueue );
        void processMessage( int clientId, const string& msg );
        /**
         * get the registered service by method. The service is registered by Export() method
         * @param method
         * @return the Service if it is exported by Export() call, NULL if the service is not exported
         */
        Service* getService( const MethodDescriptor* method );
        void requestReceived( int clientId, const string& callId, const MethodDescriptor* method, Message* requestMsg );
        void cancelRequest( int clientId, const string& callId );
        void sendResponse( int clientId, const string& callId, RpcController* controller, Message* responseMsg );
        void addRequest( Request* request );
        Request* removeRequest( int clientId, const string& callId );
        void requestFinished( Request* request );
    protected:
        int getProcessingRequests() const {
            return processingRequests_;
        }
    private:
        map< const ServiceDescriptor*, Service* > services_;
        int processingRequests_;
        ThreadSafeMap< ClientCallId, Request*> requests_;
        Queue< ClientMsg* > clientRequestMsgs_;
        Queue< ClientMsg* > clientCancelMsgs_;
        boost::thread_group requestProcThreads_;
    };

}//end name space pbrpcpp

#endif	/* BASERPCSERVER_HPP */
