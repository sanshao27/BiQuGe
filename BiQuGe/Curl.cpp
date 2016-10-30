#include "curl.h"



Curl::Curl()
{
	curl = curl_easy_init();
}


Curl::~Curl()
{
	curl_easy_cleanup(curl);
}
