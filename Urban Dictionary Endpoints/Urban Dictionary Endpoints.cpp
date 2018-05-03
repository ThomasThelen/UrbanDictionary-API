#include "stdafx.h"
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous 


int GetCommand()
{
	int command;
	std::wcout << L"Communicate with Urban Dictionary\n";
	std::wcout << L"Press '1' to define a word.\n";
	std::wcout << L"Press '2' to get a random definition\n";
	std::cin >> command;
	return command;
}


pplx::task<web::http::http_response> CreateRequestTask(std::shared_ptr<ostream>& fileStream, std::wstring endPoint, std::wstring term=L"")
{
	// Open stream to output file.
	return fstream::open_ostream(U("definition.html")).then([=](ostream outFile)
	{
		if (auto fileStream = std::make_shared<ostream>())
		{
			*fileStream = outFile;
		}

		// Start by creating an instance of the http_client object
		http_client client(U("http://api.urbandictionary.com/v0/"));
		// Define the end of the endpoint by using the builder object
		auto queryUrl = std::wstring(endPoint).append(term);
		auto builder = uri_builder(queryUrl);

		// Create the request object, set the request method, and set the request uri.
		auto request = http_request(methods::GET);
		request.set_request_uri(builder.to_string());

		// Send off the request
		return  client.request(request);
	});
	//return requestTask;
}

void HandlePostRequest(Concurrency::task<web::http::http_response>& task, std::shared_ptr<ostream>& fileStream)
{
	task.then([=](http_response response)
	{
		// Write response body into the file.
		return response.body().read_to_end(fileStream->streambuf());
	})

		// Close the file stream.
		.then([=](size_t)
	{
		return fileStream->close();
	});

	// Wait for all the outstanding I/O to complete and handle any exceptions
	try
	{
		task.wait();
	}
	catch (const std::exception &e)
	{
		printf("Error exception:%s\n", e.what());
	}
}

void DefineWord()
{
	auto term = std::wstring();
	std::wcout << L"Enter the search term:\n";
	std::wcin >> term;

	auto fileStream = std::make_shared<ostream>();
	auto task = CreateRequestTask(fileStream, L"define?term=", term);
	// Handle response headers arriving.

	task.then([=](http_response response)
	{
		// Write response body into the file.
		return response.body().read_to_end(fileStream->streambuf());
	})

		// Close the file stream.
		.then([=](size_t)
	{
		return fileStream->close();
	});

	// Wait for all the outstanding I/O to complete and handle any exceptions
	try
	{
		task.wait();
	}
	catch (const std::exception &e)
	{
		printf("Error exception:%s\n", e.what());
	}

	//HandlePostRequest(requestTask, fileStream);
	
}

void RandomWord()
{
	auto fileStream = std::make_shared<ostream>();
	auto requestTask = CreateRequestTask(fileStream, L"random");
	HandlePostRequest(requestTask, fileStream);
}

int main(int argc, char* argv[])
{
	while(1)
	{
		switch (GetCommand())
		{
		case 1:
			DefineWord();
			return 0;
		case 2:
			RandomWord();
			return 0;
		default:
			std::cout << "Command not recognized\n";
			break;
		}
	}
	return 0;
}
