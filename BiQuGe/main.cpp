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
	curl_easy_setopt(curl, CURLOPT_URL, url);//url��ַCURLOPT_WRITEFUNCTION
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true); //��ʾfollow���������ص��ض�����Ϣ��
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);  //���÷��ʵĳ�ʱ 
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true); //��ʾfollow���������ص��ض�����Ϣ��
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

		curl_easy_setopt(curl, CURLOPT_URL, url);//url��ַ 
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); //�Է��ص����ݽ��в����ĺ�����ַ  
		curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1); //�����̴�����Ϻ�ǿ�ƹرջỰ�����ٻ��湩����  
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);  //���÷��ʵĳ�ʱ  
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0"); //HTTPͷ��User-Agent��ֵ   
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

void thread_main(int num)
{
	CURL * curl;
	string url;
	string html;
	string content;
	novel_chapter_name novel_chapter_name;
	vec_nov_cha novel_chapter_list;



	//curl��ʼ��
	curl = curl_easy_init();
	if (!curl) return;

	file_m.lock();
	size_t i = init_i(num);
	file_m.unlock();

	for (; i < 23757; i += 10)
	{
		file_m.lock();
		update_log(num, i);
		file_m.unlock();

		url = host + "book/" + intTostring(i) + "/";

		//��ȡС˵Ŀ¼ҳ��
		if (!get_filecontents(curl, url.c_str(), html))
			continue;

		//��ȡС˵��
		if (!get_novel_name(html, novel_chapter_name.novel_name))
			continue;

		//��ȡС˵Ŀ¼
		if (!get_novel_chapter_list(html, novel_chapter_list, url))
			continue;

		//��ȡ��ǰС˵�����½�
		
		mysql_m.lock();
		size_t j = init_j(mysql, get_novel_id_from_database(mysql, novel_chapter_name.novel_name));
		mysql_m.unlock();

		//ץȡС˵�½�
		for (; j < novel_chapter_list.size(); j++)
		{
			//��ȡ�½���
			novel_chapter_name.chapter_name = novel_chapter_list[j].chapter_name;

			//��ȡС˵Ŀ¼ҳ�� �ɹ������ɸѡС˵�½�
			if (!get_filecontents(curl, novel_chapter_list[j].chapter_url.c_str(), html)
				|| !get_novel_chapter(html, novel_chapter_name.chapter_name, content))
			{
				content = "����δ��ץȡ�����Ķ���<a href=\"" 
							+ novel_chapter_list[j].chapter_url
							+ "\"><button class=\"btn btn-success\">����</button></a>";
			}

			//vec_m.lock();
			//novel_chapter_name_vec[num - 1] = novel_chapter_name;
			//vec_m.unlock();

			out_m.lock();
			//system("cls");
			//for (size_t k = 0; k < novel_chapter_name_vec.size(); k++)
			//{
				cout << setw(20) << novel_chapter_name.novel_name << "\t" 
					 << novel_chapter_name.chapter_name << endl;
			//}
			out_m.unlock();

			mysql_m.lock();
			if (insert_chapter_into_database(mysql, novel_chapter_name.novel_name, novel_chapter_name.chapter_name, content, novel_chapter_list[j].chapter_url, j))
			{
				insert_chapter_into_database(mysql, novel_chapter_name.novel_name, novel_chapter_name.chapter_name, content, novel_chapter_list[j].chapter_url, j);
			}
			mysql_m.unlock();
		}
	}

	//�ر�curl���
	curl_close(curl);
}


int main()
{
	vector<thread*> pthread;

	/*---���ݿ���Ϣ---*/
	char* host = "localhost";
	char* databasename = "lichaoxi";
	char* user = "root";
	char* passwd = "123456789";
	int port = 3307;

	//���ݿ�����
	if (!ConnectDatabase(mysql, host, port, databasename, user, passwd))
	{
		cout << "���ݿ�����ʧ�ܣ�";
		return 0;
	}
	
	for (size_t i = 0; i < 10; i++)
	{
		thread* p = new thread(thread_main, i + 1);
		pthread.push_back(p);
	}

	for (size_t i = 0; i < 10; i++)
	{
		pthread[i]->join();
	}

	for (size_t i = 0; i < 10; i++)
	{
		delete pthread[i];
	}

	return 0;
}