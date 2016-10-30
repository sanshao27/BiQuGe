#pragma once


#include <stdio.h>
#include <string>
#include "curl\curl.h"

using namespace std;

#pragma comment(lib, "libcurl.lib")

class Curl
{
private: 
	CURL * curl;

public:
	string html;
	Curl();
	~Curl();
};
