#include <iostream>
#include <map>
#include <string>

#include <one_of/one_of.h>

// --------------------------------------------------------------------------
//                  Define the Request "OneOf" type
// --------------------------------------------------------------------------

// Data structure for authentication information
struct AuthenticationData
{
    std::string username;
    std::string password;
};

// A request that requires authentication, contains the session token and some data
template<typename T>
class AuthentifiedRequest
{       
public:
    T data;
    std::string session_token;
};

struct Empty{};

// Define the Request type with various request kinds
ONE_OF_CREATE_ALTERNATIVE(SIGN_IN,      AuthenticationData)                 // Signin request, contains username and password
ONE_OF_CREATE_ALTERNATIVE(LOGIN,        AuthenticationData)                 // Login request, contains username and password
ONE_OF_CREATE_ALTERNATIVE(LOGOUT,       AuthentifiedRequest<Empty>)         // Logout request, no additional data
ONE_OF_CREATE_ALTERNATIVE(POST_MESSAGE, AuthentifiedRequest<std::string>)   // Post message request, contains the message to post
typedef one_of::OneOf<SIGN_IN, LOGIN, LOGOUT, POST_MESSAGE> Request;

// --------------------------------------------------------------------------
//                  Define the Response "OneOf" type
// --------------------------------------------------------------------------

// Data structure for error information
struct ErrorData
{
    int code;
    std::string message;
};

// Define the Response type with various response kinds
ONE_OF_CREATE_ALTERNATIVE(AUTH_OK,  std::string)    // Response to a successful login or sign-in, contains a session token
ONE_OF_CREATE_ALTERNATIVE(OK,       Empty)          // Generic OK response
ONE_OF_CREATE_ALTERNATIVE(ERROR,    ErrorData)      // Error response, contains an error code and message
typedef one_of::OneOf<AUTH_OK, OK, ERROR> Response;

// --------------------------------------------------------------------------
//              Create a simple server to handle requests
// --------------------------------------------------------------------------

class Server
{
public:

    // Handle incoming requests and produce appropriate responses
    Response handleRequest(const Request& request)
    {
        Response response(ERROR{}, ErrorData{1, "Internal error"});

        request.match<SIGN_IN>([&](const AuthenticationData& req)
        {
            if(user_database.find(req.username) != user_database.end()) {
                response.emplace<ERROR>(ErrorData{2, "User already exists"});
                return;
            }
            user_database[req.username] = req.password;
            std::string session_token = "token_" + req.username; // Simplified token generation
            active_sessions[session_token] = req.username;
            response.emplace<AUTH_OK>(session_token);
        })
        .match<LOGIN>([&](const AuthenticationData& req)
        {
            auto it = user_database.find(req.username);
            if(it == user_database.end() || it->second != req.password) {
                response.emplace<ERROR>(ErrorData{3, "Invalid credentials"});
                return;
            }
            std::string session_token = "token_" + req.username; // Simplified token generation
            active_sessions[session_token] = req.username;
            response.emplace<AUTH_OK>(session_token);
        })
        .match<LOGOUT>([&](const AuthentifiedRequest<Empty>& req)
        {
            auto it = active_sessions.find(req.session_token);
            if(it == active_sessions.end()) {
                response.emplace<ERROR>(ErrorData{4, "Invalid session"});
                return;
            }
            active_sessions.erase(it);
            response.emplace<OK>();
        })
        .match<POST_MESSAGE>([&](const AuthentifiedRequest<std::string>& req)
        {
            auto it = active_sessions.find(req.session_token);
            if(it == active_sessions.end()) {
                response.emplace<ERROR>(ErrorData{4, "Invalid session"});
                return;
            }
            message_database[it->second] = req.data;
            response = OK();
        })
        .assertMatchIsExhaustive();

        return response;
    }

private:

    std::map<std::string, std::string> user_database;       // username -> password
    std::map<std::string, std::string> message_database;    // username -> message
    std::map<std::string, std::string> active_sessions;     // session_token -> username
};

// --------------------------------------------------------------------------
//                Test the server with various requests
// --------------------------------------------------------------------------

int main()
{
    Server server;

    // --------------------------------------------------------------------------
    // Simulate a SIGN_IN request   
    // --------------------------------------------------------------------------

    std::string session_token = "";

    server.handleRequest(Request(SIGN_IN{}, AuthenticationData{"user1", "password1"}))
    .match<AUTH_OK>([&](const std::string& token)
    {
        std::cout << "SIGN_IN successful, session token: " << token << std::endl;
        session_token = token;
    })
    .match<ERROR>([&](const ErrorData& error)
    {
        std::cout << "SIGN_IN failed, error " << error.code << ": " << error.message << std::endl;
    })
    .fallback([]()
    {
        std::cout << "Unexpected response type" << std::endl;
    });

    if(session_token.empty()) { return 1; } // Exit if sign-in failed

    // --------------------------------------------------------------------------
    // Simulate a LOGIN request with wrong credentials   
    // --------------------------------------------------------------------------

    server.handleRequest(Request(LOGIN{}, AuthenticationData{"user1", "wrong_password"}))
    .match<ERROR>([&](const ErrorData& error) 
    {
        std::cout << "LOGIN with the wrong password failed with error " << error.code << ": " << error.message << std::endl;
    })
    .fallback([]()
    {
        std::cout << "Unexpected response type" << std::endl;
    });

    // --------------------------------------------------------------------------
    // Simulate a POST_MESSAGE request   
    // --------------------------------------------------------------------------        

    server.handleRequest(Request(POST_MESSAGE{}, AuthentifiedRequest<std::string>{"Hello, World!", session_token}))
    .match<OK>([&](const Empty&)
    {
        std::cout << "POST_MESSAGE successful" << std::endl;
    })
    .match<ERROR>([&](const ErrorData& error) 
    {
        std::cout << "POST_MESSAGE failed, error " << error.code << ": " << error.message << std::endl;
    })
    .fallback([]()
    {
        std::cout << "Unexpected response type" << std::endl;
    });

    // --------------------------------------------------------------------------
    // Simulate a LOGOUT request    
    // --------------------------------------------------------------------------

    server.handleRequest(Request(LOGOUT{}, AuthentifiedRequest<Empty>{{}, session_token}))
    .match<OK>([&](const Empty&)
    {
        std::cout << "LOGOUT successful" << std::endl;
    })
    .match<ERROR>([&](const ErrorData& error)
    {
        std::cout << "LOGOUT failed, error " << error.code << ": " << error.message << std::endl;
    })
    .fallback([]()
    {
        std::cout << "LOGOUT response type" << std::endl;
    });
}