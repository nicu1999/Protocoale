#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "json.hpp"
#include "helpers.h"

using namespace nlohmann;
using namespace std;

struct Back
{
	int code;
	json payload;
	
};

class Current
{
private:
	Back * processResponse(string responseRaw, vector<string> lines)
	{
		Back *response = new Back();
		try {
			response->payload = json::parse(lines.back()); 
		} catch (...) {}
		lines = tokenizer(responseRaw, '\n');
		response->code = stoi(tokenizer(lines[0])[1]);
		return response;
	}
	
	Back *parseResponse(string responseRaw)
	{
		vector<string> lines = tokenizer(responseRaw, '\n');
		Back *response =processResponse(responseRaw, lines);
		for (auto &x : lines) {
			if (x.find("Set-Cookie: ") != string::npos) {
				x.erase(0, strlen("Set-Cookie: "));
				vector<string> temp = tokenizer(tokenizer(x, ';')[0], '=');
				cookies[temp[0]] = temp[1];
			}
		}

		return response;
	}
	
	vector<string> tokenizer(const string& str, char delim = ' ')
	{
		vector<string> tkns;
		stringstream ss(str);
		string tkn;

		while (getline(ss, tkn, delim)) {
			tkns.push_back(tkn);
		}

		return tkns;
	}

	
	char *ipConverter(const string& host) {
	hostent* hostname = gethostbyname(host.c_str());
	if(hostname)
		return inet_ntoa(**(in_addr**)hostname->h_addr_list);
	return {};
}
public:
	map<string, string> cookies;
	string tkn;

	Current() {};
	~Current() {};
	
	char *Request(string host, string httpRequest)
	{
		int sockfd = open_connection(ipConverter(host), 8080, AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
			return NULL;
		send_to_server(sockfd, httpRequest.c_str());
		char *response = receive_from_server(sockfd);
		close(sockfd);
		return response;
	}
	
	Back *get(string host, string endpoint) {
		string httpRequest;

		httpRequest = "GET " + endpoint + " HTTP/1.1\r\n";
		httpRequest = httpRequest + "Host: " + host + "\r\n";

		if (!tkn.empty()){
			httpRequest = httpRequest + "Authorization: Bearer " + tkn + "\r\n";
		}

		if (!cookies.empty()) {
			string cookie = "Cookie: ";
			for (const auto &x : cookies) {
				if (cookie != "Cookie: ") {
					cookie = cookie + ';';	
				}
				cookie = cookie + x.first + "=" + x.second;
			}
			httpRequest = httpRequest + cookie + "\r\n";
		}
		httpRequest = httpRequest + "\r\n";
		return parseResponse(string(Request(host, httpRequest)));
}
	string httpFormat(string host, string endpoint, map<string, string> args)
	{
		string httpRequest;
		json payload = args;

		httpRequest = "POST " + endpoint + " HTTP/1.1\r\n";
		httpRequest = httpRequest + "Host: " + host + "\r\n";
		httpRequest = httpRequest + "Content-Type: application/json\r\n";
		httpRequest = httpRequest + "Content-Length: " + to_string(string(payload.dump()).size()) + "\r\n";

		if (!tkn.empty()){
			httpRequest = httpRequest + "Authorization: Bearer " + tkn + "\r\n";
		}

		if (!cookies.empty()) {
			string cookie = "Cookie: ";
			for (const auto &x : cookies) {
				if (cookie != "Cookie: ") {
					cookie = cookie + ';';	
				}
				cookie = cookie + x.first + "=" + x.second;
			}
			httpRequest = httpRequest + cookie + "\r\n";
		}

		httpRequest = httpRequest + "\r\n";
		httpRequest = httpRequest + payload.dump() + "\r\n\r\n";
		return httpRequest;
	}
	
	Back *post(string host, string endpoint, map<string, string> args)
	{
		string httpRequest = httpFormat(host, endpoint, args);
		return parseResponse(string(Request(host, httpRequest)));
	}
	
	Back * delt(string host, string endpoint) {
		string httpRequest;

		httpRequest = "DELETE " + endpoint + " HTTP/1.1\r\n";
		httpRequest = httpRequest + "Host: " + host + "\r\n";

		if (!tkn.empty()){
			httpRequest = httpRequest + "Authorization: Bearer " + tkn + "\r\n";
		}

		if (!cookies.empty()) {
			string cookie = "Cookie: ";
			for (const auto &x : cookies) {
				if (cookie != "Cookie: ") {
					cookie = cookie + ';';	
				}
				cookie = cookie + x.first + "=" + x.second;
			}
			httpRequest = cookie + "\r\n";
		}
		httpRequest = httpRequest + "\r\n";
		return parseResponse(string(Request(host, httpRequest)));
	}
	
};
