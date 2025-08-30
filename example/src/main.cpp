#include <iostream>
#include <optional>
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

// Define the Request type with various request kinds
GENERATE_ONE_OF(Request,
    (SIGN_IN,  AuthenticationData),                     // Signin request, contains username and password
    (LOGIN,  AuthenticationData),                       // Login request, contains username and password
    (LOGOUT, AuthentifiedRequest<Request::Empty>),      // Logout request, no additional data
    (POST_MESSAGE, AuthentifiedRequest<std::string>))   // Post message request, contains the message to post

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
GENERATE_ONE_OF(Response,
    (AUTH_OK, std::string),     // Response to a successful login or sign-in, contains a session token
    (OK,  Response::Empty),     // Generic OK response
    (ERROR, ErrorData))         // Error response, contains an error code and message

// --------------------------------------------------------------------------
//              Create a simple server to handle requests
// --------------------------------------------------------------------------

class Server
{
public:

    // Handle incoming requests and produce appropriate responses
    Response handleRequest(Request req)
    {
        Request::Visitor<Response> visitor([](const Request::Keys& key) 
        {
            // Fallback for unhandled request types
            return Response::create<Response::Keys::ERROR>(ErrorData{100, "No handler for request"});
        });
        
        // ---------------------- SIGN_IN Request ------------------------
        visitor.SIGN_IN = [this](const AuthenticationData& auth) -> Response 
        {
            if(user_database.find(auth.username) != user_database.end()) {
                return Response::create<Response::Keys::ERROR>(ErrorData{1, "User already exists"});
            }
            user_database[auth.username] = auth.password;
            std::string session_token = "token_" + auth.username; // Simplified token generation
            active_sessions[session_token] = auth.username;
            return Response::create<Response::Keys::AUTH_OK>(session_token);
        };

        // ---------------------- LOGIN Request --------------------------
        visitor.LOGIN = [this](const AuthenticationData& auth) -> Response 
        {
            auto it = user_database.find(auth.username);
            if(it == user_database.end() || it->second != auth.password) {
                return Response::create<Response::Keys::ERROR>(ErrorData{2, "Invalid credentials"});
            }
            std::string session_token = "token_" + auth.username; // Simplified token generation
            active_sessions[session_token] = auth.username;
            return Response::create<Response::Keys::AUTH_OK>(session_token);
        };

        // ---------------------- LOGOUT Request -------------------------
        visitor.LOGOUT = [this](const AuthentifiedRequest<Request::Empty>& auth_req) -> Response 
        {
            auto it = active_sessions.find(auth_req.session_token);
            if(it == active_sessions.end()) {
                return Response::create<Response::Keys::ERROR>(ErrorData{3, "Invalid session"});
            }
            active_sessions.erase(it);
            return Response::create<Response::Keys::OK>(Response::Empty{});
        };

        // ---------------------- POST_MESSAGE Request -------------------
        visitor.POST_MESSAGE = [this](const AuthentifiedRequest<std::string>& auth_req) -> Response 
        {
            auto it = active_sessions.find(auth_req.session_token);
            if(it == active_sessions.end()) {
                return Response::create<Response::Keys::ERROR>(ErrorData{3, "Invalid session"});
            }
            message_database[it->second] = auth_req.data;
            return Response::create<Response::Keys::OK>(Response::Empty{});
        };

        return req.visit(visitor);
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

    Request sign_in_req = Request::create<Request::Keys::SIGN_IN>(AuthenticationData{"user1", "password1"});
    Response sign_in_res = server.handleRequest(sign_in_req);
    Response::Visitor<std::optional<std::string>> sign_in_visitor([](const Response::Keys& key)
    {
        std::cout << "Unexpected response type" << std::endl;
        return std::nullopt;
    }); 

    // ---------------------- AUTH_OK ------------------------
    sign_in_visitor.AUTH_OK = [&](const std::string& token) 
    {
        std::cout << "SIGN_IN successful, session token: " << token << std::endl;
        return token;
    };
    // ---------------------- ERROR --------------------------
    sign_in_visitor.ERROR = [&](const ErrorData& error) 
    {
        std::cout << "SIGN_IN failed, error " << error.code << ": " << error.message << std::endl;
        return std::nullopt;
    };

    std::optional<std::string> session_token = sign_in_res.visit(sign_in_visitor);
    if(!session_token) { return 1; } // Exit if sign-in failed

    // --------------------------------------------------------------------------
    // Simulate a LOGIN request with wrong credentials   
    // --------------------------------------------------------------------------

    Request failed_login_req = Request::create<Request::Keys::LOGIN>(AuthenticationData{"user1", "wrongpassword"});
    Response failed_login_res = server.handleRequest(failed_login_req);
    Response::Visitor<void> failed_login_visitor([](const Response::Keys& key) 
    {
        std::cout << "Unexpected response type" << std::endl;
    });
    failed_login_visitor.AUTH_OK = [&](const std::string& token) 
    {
        std::cout << "LOGIN successful, session token: " << token << std::endl;
    };
    failed_login_visitor.ERROR = [&](const ErrorData& error) 
    {
        std::cout << "LOGIN failed, error " << error.code << ": " << error.message << std::endl;
    };
    failed_login_res.visit(failed_login_visitor);

    // --------------------------------------------------------------------------
    // Simulate a POST_MESSAGE request   
    // --------------------------------------------------------------------------        

    Request post_msg_req = Request::create<Request::Keys::POST_MESSAGE>(AuthentifiedRequest<std::string>{"Hello, World!", session_token.value()});
    Response post_msg_res = server.handleRequest(post_msg_req);
    Response::Visitor<void> post_msg_visitor([](const Response::Keys& key) 
    {
        std::cout << "Unexpected response type" << std::endl;
    });
    post_msg_visitor.OK = [&](const Response::Empty&) 
    {
        std::cout << "POST_MESSAGE successful" << std::endl;
    };
    post_msg_visitor.ERROR = [&](const ErrorData& error) 
    {
        std::cout << "POST_MESSAGE failed, error " << error.code << ": " << error.message << std::endl;
    };
    post_msg_res.visit(post_msg_visitor);

    // --------------------------------------------------------------------------
    // Simulate a LOGOUT request    
    // --------------------------------------------------------------------------

    Request logout_req = Request::create<Request::Keys::LOGOUT>(AuthentifiedRequest<Request::Empty>{{}, session_token.value()});
    Response logout_res = server.handleRequest(logout_req);
    Response::Visitor<void> logout_visitor([](const Response::Keys& key) 
    {
        std::cout << "Unexpected response type" << std::endl;
    });
    logout_visitor.OK = [&](const Response::Empty&) 
    {
        std::cout << "LOGOUT successful" << std::endl;
    };
    logout_visitor.ERROR = [&](const ErrorData& error) 
    {
        std::cout << "LOGOUT failed, error " << error.code << ": " << error.message << std::endl;
    };
    logout_res.visit(logout_visitor);
}