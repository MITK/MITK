#include "mitkRESTServer.h"

#include <mitkCommon.h>

mitk::RESTServer::RESTServer()
{
}

mitk::RESTServer::RESTServer(utility::string_t url) : m_listener(url)
{
  m_listener.support(methods::GET, std::bind(&RESTServer::handle_get, this, std::placeholders::_1));
  m_listener.support(methods::PUT, std::bind(&RESTServer::handle_put, this, std::placeholders::_1));
  m_listener.support(methods::POST, std::bind(&RESTServer::handle_post, this, std::placeholders::_1));
  m_listener.support(methods::DEL, std::bind(&RESTServer::handle_delete, this, std::placeholders::_1));
}

mitk::RESTServer::~RESTServer()
{
}

void mitk::RESTServer::handle_error(pplx::task<void>& t)
{
  try
  {
    t.get();
  }
  catch (...)
  {
    mitkThrow() << "An error occured.";
  }
}


//
// Get Request 
//
void mitk::RESTServer::handle_get(http_request message)
{
  ucout << message.to_string() << endl;

  auto paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));

  message.relative_uri().path();

  concurrency::streams::fstream::open_istream(U("static/index.html"), std::ios::in).then([=](concurrency::streams::istream is)
  {
    message.reply(status_codes::OK, is, U("text/html"))
      .then([](pplx::task<void> t)
    {
      try 
      {
        t.get();
      }
      catch (...) 
      {
        mitkThrow() << "An error occured.";
      }
    });
  }).then([=](pplx::task<void>t)
  {
    try 
    {
      t.get();
    }
    catch (...) 
    {
      message.reply(status_codes::InternalError, U("INTERNAL ERROR "));
    }
  });

  return;

};

//
// A POST request
//
void mitk::RESTServer::handle_post(http_request message)
{
  ucout << message.to_string() << endl;


  message.reply(status_codes::OK, message.to_string());
  return;
};

//
// A DELETE request
//
void mitk::RESTServer::handle_delete(http_request message)
{
  ucout << message.to_string() << endl;

  utility::string_t rep = U("WRITE YOUR OWN DELETE OPERATION");
  message.reply(status_codes::OK, rep);
  return;
};


//
// A PUT request 
//
void mitk::RESTServer::handle_put(http_request message)
{
  ucout << message.to_string() << endl;
  utility::string_t rep = U("WRITE YOUR OWN PUT OPERATION");
  message.reply(status_codes::OK, rep);
  return;
};