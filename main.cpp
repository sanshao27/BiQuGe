#include <stdio.h>
#include "curl\curl.h"
#include "Function.h"
#include <iostream>
#include <thread>
#include <mutex>



#pragma comment(lib, "libcurl.lib")

typedef struct C_novel_chapter_name
{
	string novel_name;
	string chapter_name;
}novel_chapter_name;

mutex out_m, vec_m, mysql_m, file_m;

MYSQL mysql;

string host = "http://www.qu.la/";

std::string * strtmp;

size_t start;
size_t end_t;
size_t k;

size_t write_data(void* buffer, size_t size, size_t nmemb, void *stream) 
{
	*(string*)stream += (char*)buffer;
	return nmemb;
}

bool curl_init(CURL *curl)
{
	curl = curl_easy_init();
	if (!curl) return false;
	return true;
}

bool get_filecontents(CURL *curl, const char *url, string &html)
{
	html = "";
	if (!curl) return false;
	curl_easy_setopt(curl, CURLOPT_URL, url);//url地址CURLOPT_WRITEFUNCTION
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true); //表示follow服务器返回的重定向信息。
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);  //设置访问的超时 
	if (CURLE_OK != curl_easy_perform(curl)) return false;
	return true;
}

void curl_close(CURL *curl)
{
	curl_easy_cleanup(curl);
}

bool get_html(const char *url, string &html)
{
	CURL *curl;
	curl = curl_easy_init();
	if (curl) {

		curl_easy_setopt(curl, CURLOPT_URL, url);//url地址 
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); //对返回的数据进行操作的函数地址  
		curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1); //当进程处理完毕后强制关闭会话，不再缓存供重用  
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);  //设置访问的超时  
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0"); //HTTP头中User-Agent的值   
		if (CURLE_OK != curl_easy_perform(curl))
		{
			curl_easy_cleanup(curl);
			return false;
		}
		else
		{
			curl_easy_cleanup(curl);
			return true;
		}
	}
	else
	{
		return false;
	}
}

void thread_main(size_t num)
{
	CURL * curl;
	string url;
	string html;
	string content;
	novel_chapter_name novel_chapter_name;
	vec_nov_cha novel_chapter_list;

	//curl初始化
	curl = curl_easy_init();
	if (!curl) return;

	for (size_t i = start + num; i < end_t; i += k)
	{

		url = host + "book/" + intTostring((int)i) + "/";

		//获取小说目录页面
		if (!get_filecontents(curl, url.c_str(), html))
			continue;

		//获取小说名
		if (!get_novel_name(html, novel_chapter_name.novel_name))
			continue;

		//获取小说目录
		if (!get_novel_chapter_list(html, novel_chapter_list, url))
			continue;

		//获取当前小说最新章节
		
		mysql_m.lock();
		size_t j = (size_t)init_j(mysql, get_novel_id_from_database(mysql, novel_chapter_name.novel_name));
		mysql_m.unlock();

		//抓取小说章节
		for (; j < novel_chapter_list.size(); j++)
		{
			//获取章节名
			novel_chapter_name.chapter_name = novel_chapter_list[j].chapter_name;

			//获取小说目录页面 成功则从中筛选小说章节
			if (!get_filecontents(curl, novel_chapter_list[j].chapter_url.c_str(), html)
				|| !get_novel_chapter(html, novel_chapter_name.chapter_name, content))
			{
				content = "本章未能抓取到，阅读请<a href=\"" 
							+ novel_chapter_list[j].chapter_url
							+ "\"><button class=\"btn btn-success\">点我</button></a>";
			};

			out_m.lock();
				cout << setw(20) << novel_chapter_name.novel_name << "\t" 
					 << novel_chapter_name.chapter_name << endl;
			out_m.unlock();

			mysql_m.lock();
			if (insert_chapter_into_database(mysql, novel_chapter_name.novel_name, novel_chapter_name.chapter_name, content, novel_chapter_list[j].chapter_url, (int)j))
			{
				insert_chapter_into_database(mysql, novel_chapter_name.novel_name, novel_chapter_name.chapter_name, content, novel_chapter_list[j].chapter_url, (int)j);
			}
			mysql_m.unlock();
		}
	}

	//关闭curl句柄
	curl_close(curl);
}


int main()
{
	vector<thread*> pthread;

	/*---数据库信息---*/
	char* host = "localhost";
	char* databasename = "lichaoxi";
	char* user = "root";
	char* passwd = "123456789";
	int port = 3307;

	//数据库连接
	if (!ConnectDatabase(mysql, host, port, databasename, user, passwd))
	{
		cout << "数据库连接失败！";
		return 0;
	}

	cout << "请输入起始位置(0/10000/16000)：";
	cin >> start;
	cout << "请输入结束位置(10000/16000/23757)";
	cin >> end_t;
	cout << "线程数：";
	cin >> k;


	
	for (size_t i = 0; i < k; i++)
	{
		thread* p = new thread(thread_main, i + 1);
		pthread.push_back(p);
	}

	for (size_t i = 0; i < k; i++)
	{
		pthread[i]->join();
	}

	for (size_t i = 0; i < k; i++)
	{
		delete pthread[i];
	}

	return 0;
}