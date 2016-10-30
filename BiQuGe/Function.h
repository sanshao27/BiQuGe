#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <mysql.h>
using namespace std;

typedef struct C_novel_info
{
	string novel_image;
	string novel_category;
	string novel_status;
	string novel_name;
	string novel_author;
	string novel_author_link;
	string update_time;
	string novel_description;
	string novel_url;
	bool TorF;
}novel_info;

typedef struct C_novel_chapter
{
	string chapter_url;
	string chapter_name;
}novel_chapter;

typedef vector<novel_chapter> vec_nov_cha;

/*----------------普适函数------------------------*/

//int转string
string intTostring(int i)
{
	string temp;
	stringstream aa;
	aa << i; aa >> temp;
	return temp;
}

//string转int
int stringToint(string i)
{
	int temp;
	stringstream aa;
	aa << i; aa >> temp;
	return temp;
}

//去除首尾空格
string& trim(string &s)
{
	if (s.empty())
	{
		return s;
	}
	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}

//获得当前时间
string get_str_time()
{
	char now[64];
	time_t tt;
	struct tm ttime = { 0 };
	tt = time(&tt);
	localtime_s(&ttime, &tt);
	strftime(now, 64, "%Y-%m-%d %H:%M:%S", &ttime);
	return now;
}

//获取html中pos之后指定字符间的字符
bool get_info(const char* start, const char* end, string &html, string &info, int &pos)
{
	info = "";
	if (pos < 0) pos = 0;
	int indexs;
	indexs = html.find(start, pos);
	if (indexs == -1)return false;
	pos = html.find(end, indexs + strlen(start));
	if (pos == -1)return false;
	info = html.substr(indexs + strlen(start), pos - indexs - strlen(start));
	return true;
}

//获取html中指定字符间的字符
bool get_info(const char* start, const char* end, string &html, string &info)
{
	info = "";
	int pos = 0;
	int indexs;
	indexs = html.find(start, pos);
	if (indexs == -1)return false;
	pos = html.find(end, indexs + strlen(start));
	if (pos == -1)return false;
	info = html.substr(indexs + strlen(start), pos - indexs - strlen(start));
	return true;
}

//连接数据库
bool ConnectDatabase(MYSQL &mysql, char * host, int port, char * databasename, char * user, char * passwd)
{
	//初始化mysql指针
	if (mysql_init(&mysql) == NULL)
	{
		return false;
	}
	//连接数据库
	if (mysql_real_connect(&mysql, host, user, passwd, databasename, port, NULL, 0) == NULL)
	{
		return false;
	}
	//设置数据库字符集
	if (mysql_set_character_set(&mysql, "GBK") != 0)
	{
		return false;
	}
	return true;
}

//从数据库中取出数据
string SelectData(MYSQL &mysql, const char * SQL, int num)
{
	MYSQL_ROW m_row;
	MYSQL_RES *m_res;
	int rnum = 0;
	char cg = ' ';//字段隔开
	if (mysql_query(&mysql, SQL) != 0)
	{
		return "";
	}
	m_res = mysql_store_result(&mysql);

	if (m_res == NULL)
	{
		return "";
	}
	//写入找到的信息到str中
	string str("");
	while (m_row = mysql_fetch_row(m_res))
	{
		for (int i = 0; i < num; i++)
		{
			if (m_row[i] == NULL) return "";
			str += m_row[i];
			str += cg;
		}
		rnum++;
	}
	mysql_free_result(m_res);
	return str;
}

/*---------------特用函数-------------------*/

//从数据库中取出小说id
int get_novel_id_from_database(MYSQL &mysql, string& novel_name)
{
	return stringToint(SelectData(mysql, ("select id from novel where novel_name = '"
		+ novel_name + "'").c_str(), 1));
}

//从html中筛选出小说列表
bool get_novel_chapter_list(string &html, vec_nov_cha& vector_novel_chapter, string novel_url)
{
	vector_novel_chapter.clear();
	novel_chapter temp;
	int pos = 0; 
	string chapter_url;
	while (get_info("<dd><a href=\"", "\">", html, temp.chapter_url, pos))
	{
		temp.chapter_name = html.substr(pos + 2, html.find("</a>", pos + 1) - pos - 2);
		temp.chapter_url = novel_url + temp.chapter_url;
		vector_novel_chapter.push_back(temp);
	}
	return true;
}

////从html中筛选出小说信息
//novel_info& get_novel_info(string &novel_url, novel_info& novel_info)
//{
//	string html; int pos = 0;
//
//	if (!get_html(novel_url, html))
//	{
//		novel_info.TorF = false;
//		return novel_info;
//	}
//
//	//get_info("og:description\" content=\"", "\"/>", html, novel_info.novel_description, pos);
//	//get_info("og:image\" content=\"", "\"/>", html, novel_info.novel_image, pos);
//	//get_info("og:novel:category\" content=\"", "\"/>", html, novel_info.novel_category, pos);
//	//get_info("og:novel:author\" content=\"", "\"/>", html, novel_info.novel_author, pos);
//	get_info("og:novel:book_name\" content=\"", "\"/>", html, novel_info.novel_name, pos);
//	//get_info("og:novel:read_url\" content=\"", "\"/>", html, novel_info.novel_url, pos);
//	//get_info("og:novel:status\" content=\"", "\"/>", html, novel_info.novel_status, pos);
//	//get_info("og:novel:author_link\" content=\"", "\"/>", html, novel_info.novel_author_link, pos);
//	//get_info("og:novel:update_time\" content=\"", "\"/>", html, novel_info.update_time, pos);
//
//	novel_info.TorF = true;
//
//	return novel_info;
//
//}

//从html中筛选出小说信息
novel_info& get_novel_info(string &html, novel_info& novel_info)
{

	if (!get_info("og:novel:book_name\" content=\"", "\"/>", html, novel_info.novel_name))
	{
		novel_info.TorF = false;
	}

	novel_info.TorF = true;

	return novel_info;

}

//从html中筛选出小说名
bool get_novel_name(string &html, string &novel_name)
{
	novel_name = "";
	return get_info("og:novel:book_name\" content=\"", "\"/>", html, novel_name);
}

//抓取小说章节
bool get_novel_chapter(string &html, string &chapter_name, string &chapter_content)
{
	//bool T1 = get_info("<h1> ", "</h1>", html, chapter_name);
	//if(!T1) T1 = get_info("<h1>", "</h1>", html, chapter_name);
	bool T2 = get_info("<script>readx();</script>", "</div>", html, chapter_content);
	return  T2;
}

//向数据库中插入小说章节
bool insert_chapter_into_database(MYSQL &mysql, string &novel_name, string &chapter_name, string &chapter_content, string chapter_url, int i)
{
	int novel_id = get_novel_id_from_database(mysql, novel_name);

	return mysql_query(&mysql, ("insert into novel_chapter_" + intTostring((novel_id + 9) / 10) + " (novel_id, chapter_id, chapter_name, chapter_content, chapter_update_time, chapter_url) values ("
								+ intTostring(novel_id) + ", "
								+ intTostring(i + 1) + ", '"
								+ chapter_name + "', '"
								+ chapter_content + "', '"
								+ get_str_time() + "', 'http://"
								+ chapter_url + "')").c_str()) != 0;
	//return mysql_query(&mysql, ("insert into novel_chapter (novel_id, chapter_id, chapter_name, chapter_content, chapter_update_time, chapter_url) values ("
	//	+ intTostring(novel_id) + ", "
	//	+ intTostring(i + 1) + ", '"
	//	+ chapter_name + "', '"
	//	+ chapter_content + "', '"
	//	+ get_str_time() + "', 'http://"
	//	+ chapter_url + "')").c_str()) != 0;
}

////读取日志断点续传获取 i
//int init_i(int id)
//{
//	ifstream fin(intTostring(id)+".txt", ios::_Nocreate);
//	if (!fin) return id;
//	int i;
//	if (fin >> i) return i;
//	return id;
//}

//查询数据断点续传获取 j
int init_j(MYSQL &mysql, int novel_id)
{
	string chapter_id = SelectData(mysql, ("select chapter_id from novel_chapter_" + intTostring((novel_id + 9) / 10) + " where novel_id = " + intTostring(novel_id)+ " order by chapter_id desc limit 1").c_str(), 1);
	return chapter_id == "" ? 0 : stringToint(chapter_id);
}

//获取最新小说序号
int init_i(int num)
{
	ifstream in("biquge.log", ios::_Nocreate);
	if (!in) return num;
	
	int i;
	vector<int> veci;
	
	for (size_t j = 0; j < 10; j++) 
	{
		if (!(in >> i)) break;
		veci.push_back(i);
	}
	in.close();
	if (veci.size() < num) return num;
	return veci[num - 1];
}

bool update_log(int num, int id)
{
	ifstream in("biquge.log", ios::_Nocreate);
	if (!in)
	{
		ofstream out("biquge.log");
		if (!out) return false;
		for (size_t i = 0; i < 10; i++)
		{
			if (i == num - 1)
				out << id << endl;
			else
				out << i + 1 << endl;
		}
		out.close();
	}
	else
	{
		int i;
		vector<int> veci;

		for (size_t j = 0; j < 10; j++)
		{
			if (!(in >> i)) break;
			veci.push_back(i);
		}

		in.close();

		for (size_t j = veci.size(); j < 10; j++)
		{
			if (j == num - 1)
				veci.push_back(id);
			else
				veci.push_back(j + 1);
		}

		ofstream out("biquge.log");
		if (!out) return false;

		for (size_t j = 0; j < 10; j++)
		{
			out << veci[j] << endl;
		}
		out.close();
	}
	return true;
}





