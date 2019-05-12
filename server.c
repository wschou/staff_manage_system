/*************************************************************************
#	 FileName	: server.c
#	 Author		: fengjunhui 
#	 Email		: 18883765905@163.com 
#	 Created	: 2018年12月29日 星期六 13时44分59秒
 ************************************************************************/

#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>

#include "common.h"

sqlite3 *db;  //仅服务器使用

int process_user_or_admin_login_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//封装sql命令，表中查询用户名和密码－存在－登录成功－发送响应－失败－发送失败响应	
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;

	msg->info.usertype =  msg->usertype;
	strcpy(msg->info.name,msg->username);
	strcpy(msg->info.passwd,msg->passwd);
	
	printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);		
	}else{
		//printf("----nrow-----%d,ncolumn-----%d.\n",nrow,ncolumn);		
		if(nrow == 0){
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd,msg,sizeof(MSG),0);
		}
	}
	return 0;	
}

int process_user_modify_request(int acceptfd,MSG *msg)
{
	char *errmsg;
	char datetime[256];
	char buff[256];
	int num;
	time_t now;
	struct tm *tm_now;
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	switch(msg->flags) {
	case 1:
		sprintf(buff, "update usrinfo set addr = '%s' where name = '%s';", msg->info.addr, msg->username);
		break;
	case 2:
		sprintf(buff, "update usrinfo set phone = '%s' where name = '%s';", msg->info.phone, msg->username);
		break;
	case 3:
		sprintf(buff, "update usrinfo set passwd = '%s' where name = '%s';", msg->info.passwd, msg->username);
		break;
	default:
		break;
	}
	if(msg->flags>=1 && msg->flags<=3) {
		printf("%s\n", buff);
		if(sqlite3_exec(db, buff, NULL,NULL,&errmsg)!= SQLITE_OK) {
			printf("%s.\n",errmsg);
			strcpy(msg->recvmsg, errmsg);
		} else {
			printf("modify user success.\n");
			strcpy(msg->recvmsg, "OK");
		}
			send(acceptfd, msg, sizeof(MSG), 0);
		/*************写日志*****************/
		time(&now);
		tm_now = localtime(&now);
		sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
		sprintf(buff, "insert into historyinfo values('%s', '%s','修改个人信息');", datetime, msg->username);
		printf("%s\n", buff);
		if(sqlite3_exec(db, buff, NULL,NULL,&errmsg)!= SQLITE_OK) {
			printf("%s.\n",errmsg);
		} else {
			printf("add log success.\n");
		}
	} else {
			printf("command error.\n");
			strcpy(msg->recvmsg, "FAIL: 命令错误");
			send(acceptfd, msg, sizeof(MSG), 0);
	}

}



int process_user_query_request(int acceptfd,MSG *msg)
{
	char *errmsg = NULL;
	char **resultp = NULL;
	char datetime[256];
	char buff[512];
	int nrow, ncolumn;
	time_t now;
	struct tm *tm_now;
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	/************按条件查找*****************/
	sprintf(buff, "select * from usrinfo where  name='%s';",  msg->username);
	if (0 != sqlite3_get_table(db, buff, &resultp, &nrow, &ncolumn, &errmsg))
	{
		fprintf(stderr, "get table: %s\n", errmsg);
		return -1;
	}
	printf("==================================================\n");
	printf("表格共%d 记录!\n", nrow);
	printf("表格共%d 列!\n", ncolumn);
	int i, j, count = 0;
	for (i = 0; i < nrow+1; i++)
	{
		memset(msg->recvmsg, 0, sizeof(msg->recvmsg));
		for (j = 0; j < ncolumn; j++)
		{
			//printf("%-10s  ", resultp[count++]);
			sprintf(buff, "%-6s  ", resultp[count++]);
			strcat(msg->recvmsg, buff);
		}
		printf("\n");
		printf("%s\n", msg->recvmsg);
		msg->flags = (i==nrow?0:1);
		if(i > 0) {
			send(acceptfd, msg, sizeof(MSG), 0);
		}
	}
	if(nrow == 0) {
		msg->flags = 0;
		send(acceptfd, msg, sizeof(MSG), 0);
	}
	printf("==================================================\n");

	/*************写日志*****************/
	time(&now);
	tm_now = localtime(&now);
	sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
	sprintf(buff, "insert into historyinfo values('%s', 'user[%s]','查询个人信息');",datetime, msg->username);
	printf("%s\n", buff);
	if(sqlite3_exec(db, buff, NULL,NULL,&errmsg)!= SQLITE_OK) {
		printf("%s.\n",errmsg);
	} else {
		printf("add log success.\n");
	}

}


int process_admin_modify_request(int acceptfd,MSG *msg)
{
	char *errmsg;
	char datetime[256];
	char buff[256];
	int num;
	time_t now;
	struct tm *tm_now;
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	switch(msg->flags) {
	case 1:
		sprintf(buff, "update usrinfo set name = '%s' where staffno = %d;", msg->info.name, msg->info.no);
		break;
	case 2:
		sprintf(buff, "update usrinfo set age = %d where staffno = %d;", msg->info.age, msg->info.no);
		break;
	case 3:
		sprintf(buff, "update usrinfo set addr = '%s' where staffno = %d;", msg->info.addr, msg->info.no);
		break;
	case 4:
		sprintf(buff, "update usrinfo set phone = '%s' where staffno = %d;", msg->info.phone, msg->info.no);
		break;
	case 5:
		sprintf(buff, "update usrinfo set work = '%s' where staffno = %d;", msg->info.work, msg->info.no);
		break;
	case 6:
		sprintf(buff, "update usrinfo set salary = %lf where staffno = %d;", msg->info.salary, msg->info.no);
		break;
	case 7:
		sprintf(buff, "update usrinfo set date = '%s' where staffno = %d;", msg->info.date, msg->info.no);
		break;
	case 8:
		sprintf(buff, "update usrinfo set level = %d where staffno = %d;", msg->info.level, msg->info.no);
		break;
	case 9:
		sprintf(buff, "update usrinfo set passwd = '%s' where staffno = %d;", msg->info.passwd, msg->info.no);
		break;
	default:
		break;
	}
	if(msg->flags>=1 && msg->flags<=9) {
		printf("%s\n", buff);
		if(sqlite3_exec(db, buff, NULL,NULL,&errmsg)!= SQLITE_OK) {
			printf("%s.\n",errmsg);
			strcpy(msg->recvmsg, errmsg);
		} else {
			printf("modify user success.\n");
			strcpy(msg->recvmsg, "OK");
		}
			send(acceptfd, msg, sizeof(MSG), 0);
		/*************写日志*****************/
		time(&now);
		tm_now = localtime(&now);
		sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
		sprintf(buff, "insert into historyinfo values('%s', '%s','修改[%d]个人信息');", datetime, msg->username, msg->info.no);
		printf("%s\n", buff);
		if(sqlite3_exec(db, buff, NULL,NULL,&errmsg)!= SQLITE_OK) {
			printf("%s.\n",errmsg);
		} else {
			printf("add log success.\n");
		}
	} else {
			printf("command error.\n");
			strcpy(msg->recvmsg, "FAIL: 命令错误");
			send(acceptfd, msg, sizeof(MSG), 0);
	}

}


int process_admin_adduser_request(int acceptfd,MSG *msg)
{
	char *errmsg = NULL;
	char **resultp = NULL;
	char datetime[256];
	char buff[256];
	int nrow, ncolumn;
	time_t now;
	struct tm *tm_now;
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	/*****************************************************/
	while(1) {
		//接受客户端响应
		//recv(acceptfd, msg, sizeof(MSG), 0);
		printf("msg->recvmsg :%s\n",msg->recvmsg);
		/*****************************************************/
		/************查找工号*****************/
		sprintf(buff, "select * from usrinfo where staffno=%d;", msg->info.no);
		printf("%s\n", buff);
		if (0 != sqlite3_get_table(db, buff, &resultp, &nrow, 
					&ncolumn, &errmsg))
		{
			fprintf(stderr, "get table: %s\n", errmsg);
			return -1;
		}
		printf("==================================================\n");
		printf("表格共%d 记录!\n", nrow);
		int i, j, count = 0;
		for (i = 0; i < nrow+1; i++)
		{
			for (j = 0; j < ncolumn; j++)
			{
				printf("%-10s  ", resultp[count++]);
			}
			printf("\n");
		}
		printf("==================================================\n");
		/*****************************************************/
		if(nrow == 0) {
			strcpy(msg->recvmsg, "OK");
			send(acceptfd, msg, sizeof(MSG), 0);
			break;
		} else {
			strcpy(msg->recvmsg, "FAIL: ID已存在!");
			send(acceptfd, msg, sizeof(MSG), 0);
		}
		recv(acceptfd, msg, sizeof(MSG), 0);
	}
	/*****************************************************/
	recv(acceptfd, msg, sizeof(MSG), 0);
	printf("No: %d\n", msg->info.no);
	printf("Type: %d\n", msg->info.usertype);
	printf("Name: %s\n", msg->info.name);
	printf("Passwd: %s\n", msg->info.passwd);
	printf("Age: %d\n", msg->info.age);
	printf("Phone: %s\n", msg->info.phone);
	printf("Addr: %s\n", msg->info.addr);
	printf("Work: %s\n", msg->info.work);
	printf("Date: %s\n", msg->info.date);
	printf("Level: %d\n", msg->info.level);
	printf("Salary: %.2f\n", msg->info.salary);
	/*****************************************************/

	sprintf(buff, "insert into usrinfo values(%d, %d, '%s', '%s', %d, '%s', '%s', '%s', '%s', %d, %.2f);", \
			msg->info.no, msg->info.usertype, msg->info.name, msg->info.passwd, msg->info.age, msg->info.phone, \
			msg->info.addr, msg->info.work, msg->info.date, msg->info.level, msg->info.salary);
	//sprintf(buff, "insert into usrinfo(staffno, name) values(%d, '%s');", msg->info.no, msg->info.name);
	printf("%s\n", buff);
	if(sqlite3_exec(db, buff, NULL,NULL,&errmsg)!= SQLITE_OK) {
		printf("%s.\n",errmsg);
		strcpy(msg->recvmsg, "errmsg");
	} else {
		printf("add user success.\n");
		strcpy(msg->recvmsg, "OK");
	}
	send(acceptfd,msg,sizeof(MSG),0);
	/*************写日志*****************/
	time(&now);
	tm_now = localtime(&now);
	sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
	sprintf(buff, "insert into historyinfo values('%s', 'admin','增加员工[%d]');", datetime, msg->info.no);
	printf("%s\n", buff);
	if(sqlite3_exec(db, buff, NULL,NULL,&errmsg)!= SQLITE_OK) {
		printf("%s.\n",errmsg);
	} else {
		printf("add log success.\n");
	}
}



int process_admin_deluser_request(int acceptfd,MSG *msg)
{
	char *errmsg = NULL;
	char **resultp = NULL;
	char datetime[256];
	char buff[256];
	int nrow, ncolumn;
	int num;
	time_t now;
	struct tm *tm_now;
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	/*****************************************************/
	//接受客户端响应
	//recv(acceptfd, msg, sizeof(MSG), 0);
	printf("msg->recvmsg :%s\n",msg->recvmsg);
	/************按条件查找*****************/
	sprintf(buff, "select * from usrinfo where staffno=%d and name='%s';", msg->info.no, msg->info.name);
	printf("%s\n", buff);
	if (0 != sqlite3_get_table(db, buff, &resultp, &nrow, 
				&ncolumn, &errmsg))
	{
		fprintf(stderr, "get table: %s\n", errmsg);
		return -1;
	}
	printf("==================================================\n");
	printf("表格共%d 记录!\n", nrow);
	int i, j, count = 0;
	for (i = 0; i < nrow+1; i++)
	{
		for (j = 0; j < ncolumn; j++)
		{
			printf("%-10s  ", resultp[count++]);
		}
		printf("\n");
	}
	printf("==================================================\n");
	/*************删除用户*****************/
	if(nrow > 0) {
		sprintf(buff, "delete from usrinfo where staffno=%d and name='%s';", msg->info.no, msg->info.name);
		printf("%s\n", buff);
		if(sqlite3_exec(db, buff, NULL,NULL,&errmsg)!= SQLITE_OK) {
			printf("%s.\n",errmsg);
			strcpy(msg->recvmsg, "errmsg");
		} else {
			strcpy(msg->recvmsg, "OK");
			printf("delete user success.\n");
		}
		send(acceptfd, msg, sizeof(MSG), 0);
		/*************写日志*****************/
		time(&now);
		tm_now = localtime(&now);
		sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
		sprintf(buff, "insert into historyinfo values('%s', 'admin','删除工号[%d]');", datetime, num);
		printf("%s\n", buff);
		if(sqlite3_exec(db, buff, NULL,NULL,&errmsg)!= SQLITE_OK) {
			printf("%s.\n",errmsg);
		} else {
			printf("add log success.\n");
		}
	} else {
		strcpy(msg->recvmsg, "FAIL");
		send(acceptfd, msg, sizeof(MSG), 0);
	}

}


int process_admin_query_request(int acceptfd,MSG *msg)
{
	char *errmsg = NULL;
	char **resultp = NULL;
	char datetime[256];
	char buff[512];
	int nrow, ncolumn;
	time_t now;
	struct tm *tm_now;
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	/************按条件查找*****************/
	if(msg->flags == 0) {
		sprintf(buff, "select * from usrinfo where  name='%s';",  msg->info.name);
	} else {
		sprintf(buff, "select * from usrinfo;");
	}
	if (0 != sqlite3_get_table(db, buff, &resultp, &nrow, &ncolumn, &errmsg))
	{
		fprintf(stderr, "get table: %s\n", errmsg);
		return -1;
	}
	printf("==================================================\n");
	printf("表格共%d 记录!\n", nrow);
	printf("表格共%d 列!\n", ncolumn);
	int i, j, count = 0;
	for (i = 0; i < nrow+1; i++)
	{
		memset(msg->recvmsg, 0, sizeof(msg->recvmsg));
		for (j = 0; j < ncolumn; j++)
		{
			//printf("%-10s  ", resultp[count++]);
			sprintf(buff, "%-6s  ", resultp[count++]);
			strcat(msg->recvmsg, buff);
		}
		printf("\n");
		printf("%s\n", msg->recvmsg);
		msg->flags = (i==nrow?0:1);
		if(i > 0) {
			send(acceptfd, msg, sizeof(MSG), 0);
		}
	}
	if(nrow == 0) {
		msg->flags = 0;
		send(acceptfd, msg, sizeof(MSG), 0);
	}
	printf("==================================================\n");

	/*************写日志*****************/
	time(&now);
	tm_now = localtime(&now);
	sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
	sprintf(buff, "insert into historyinfo values('%s', 'admin','按条件查找');", datetime);
	printf("%s\n", buff);
	if(sqlite3_exec(db, buff, NULL,NULL,&errmsg)!= SQLITE_OK) {
		printf("%s.\n",errmsg);
	} else {
		printf("add log success.\n");
	}

}

int process_admin_history_request(int acceptfd,MSG *msg)
{
	char *errmsg = NULL;
	char **resultp = NULL;
	char datetime[256];
	char buff[256];
	int nrow, ncolumn;
	time_t now;
	struct tm *tm_now;
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	/*************查询日志*****************/
	if (0 != sqlite3_get_table(db, "select * from historyinfo;", &resultp, &nrow, \
				&ncolumn, &errmsg))
	{
		fprintf(stderr, "get table: %s\n", errmsg);
		return -1;
	}
	/*************把数据发送到客户端*****************/
	printf("==================================================\n");
	printf("表格共%d 记录!\n", nrow);
	printf("表格共%d 列!\n", ncolumn);
	int i, j, count = 0;
	for (i = 0; i < nrow+1; i++)
	{
		memset(msg->recvmsg, 0, sizeof(msg->recvmsg));
		for (j = 0; j < ncolumn; j++)
		{
			sprintf(buff, "%-10s  ", resultp[count++]);
			strcat(msg->recvmsg, buff);
		}
		printf("%s\n", msg->recvmsg);
		msg->flags = (i==nrow?0:1);
		send(acceptfd, msg, sizeof(MSG), 0);

	}
	printf("==================================================\n");

	/*************写日志*****************/
	time(&now);
	tm_now = localtime(&now);
	sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
	sprintf(buff, "insert into historyinfo values('%s', 'admin','查询日志');", datetime);
	printf("%s\n", buff);
	if(sqlite3_exec(db, buff, NULL,NULL,&errmsg)!= SQLITE_OK) {
		printf("%s.\n",errmsg);
	} else {
		printf("add log success.\n");
	}

}


int process_client_quit_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);

}


int process_client_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	switch (msg->msgtype)
	{
	case USER_LOGIN:
	case ADMIN_LOGIN:
		process_user_or_admin_login_request(acceptfd,msg);
		break;
	case USER_MODIFY:
		process_user_modify_request(acceptfd,msg);
		break;
	case USER_QUERY:
		process_user_query_request(acceptfd,msg);
		break;
	case ADMIN_MODIFY:
		process_admin_modify_request(acceptfd,msg);
		break;

	case ADMIN_ADDUSER:
		process_admin_adduser_request(acceptfd,msg);
		break;

	case ADMIN_DELUSER:
		process_admin_deluser_request(acceptfd,msg);
		break;
	case ADMIN_QUERY:
		process_admin_query_request(acceptfd,msg);
		break;
	case ADMIN_HISTORY:
		process_admin_history_request(acceptfd,msg);
		break;
	case QUIT:
		process_client_quit_request(acceptfd,msg);
		break;
	default:
		break;
	}

}


int main(int argc, const char *argv[])
{
	//socket->填充->绑定->监听->等待连接->数据交互->关闭 
	int sockfd;
	int acceptfd;
	ssize_t recvbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	socklen_t cli_len = sizeof(clientaddr);

	MSG msg;
	//thread_data_t tid_data;
	char *errmsg;

	if(sqlite3_open(STAFF_DATABASE,&db) != SQLITE_OK){
		printf("%s.\n",sqlite3_errmsg(db));
	}else{
		printf("the database open success.\n");
	}

	if(sqlite3_exec(db,"create table usrinfo(staffno integer,usertype integer,name text,passwd text,age integer,phone text,addr text,work text,date text,level integer,salary REAL);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("create usrinfo table success.\n");
	}

	if(sqlite3_exec(db,"create table historyinfo(time text,name text,words text);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{ //华清远见创客学院         嵌入式物联网方向讲师
		printf("create historyinfo table success.\n");
	}

	//创建网络通信的套接字
	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("socket failed.\n");
		exit(-1);
	}
	printf("sockfd :%d.\n",sockfd); 


	/*优化4： 允许绑定地址快速重用 */
	int b_reuse = 1;
	setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof (int));

	//填充网络结构体
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	//	serveraddr.sin_port   = htons(atoi(argv[2]));
	//	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port   = htons(5001);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");


	//绑定网络套接字和网络结构体
	if(bind(sockfd, (const struct sockaddr *)&serveraddr,addrlen) == -1){
		printf("bind failed.\n");
		exit(-1);
	}

	//监听套接字，将主动套接字转化为被动套接字
	if(listen(sockfd,10) == -1){
		printf("listen failed.\n");
		exit(-1);
	}

	//定义一张表
	fd_set readfds,tempfds;
	//清空表
	FD_ZERO(&readfds);
	FD_ZERO(&tempfds);
	//添加要监听的事件
	FD_SET(sockfd,&readfds);
	int nfds = sockfd;
	int retval;
	int i = 0;

#if 0 //添加线程控制部分
	pthread_t thread[N];
	int tid = 0;
#endif

	while(1){
		tempfds = readfds;
		//记得重新添加
		retval =select(nfds + 1, &tempfds, NULL,NULL,NULL);
		printf("00\n");
		//判断是否是集合里关注的事件
		for(i = 0;i < nfds + 1; i ++){
			if(FD_ISSET(i,&tempfds)){
				if(i == sockfd){
					//数据交互 
					acceptfd = accept(sockfd,(struct sockaddr *)&clientaddr,&cli_len);
					printf("01\n");
					if(acceptfd == -1){
						printf("acceptfd failed.\n");
						exit(-1);
					}
					printf("ip : %s.\n",inet_ntoa(clientaddr.sin_addr));
					FD_SET(acceptfd,&readfds);
					nfds = nfds > acceptfd ? nfds : acceptfd;
				}else{
					recvbytes = recv(i,&msg,sizeof(msg),0);
					printf("02\n");
					printf("msg.type :%#x.\n",msg.msgtype);
					if(recvbytes == -1){
						printf("recv failed.\n");
						continue;
					}else if(recvbytes == 0){
						printf("peer shutdown.\n");
						close(i);
						FD_CLR(i, &readfds);  //删除集合中的i
					}else{
						process_client_request(i,&msg);
					}
				}
			}
		}
	}
	close(sockfd);

	return 0;
}







#if 0
//tid_data.acceptfd = acceptfd;   //暂时不使用这种方式
//tid_data.state	  = 1;
//tid_data.thread   = thread[tid++];	
//pthread_create(&tid_data.thread, NULL,client_request_handler,(void *)&tid_data);
#endif 

#if 0
void *client_request_handler(void * args)
{
	thread_data_t *tiddata= (thread_data_t *)args;

	MSG msg;
	int recvbytes;
	printf("tiddata->acceptfd :%d.\n",tiddata->acceptfd);

	while(1){  //可以写到线程里--晚上的作业---- UDP聊天室
		//recv 
		memset(msg,sizeof(msg),0);
		recvbytes = recv(tiddata->acceptfd,&msg,sizeof(msg),0);
		if(recvbytes == -1){
			printf("recv failed.\n");
			close(tiddata->acceptfd);
			pthread_exit(0);
		}else if(recvbytes == 0){
			printf("peer shutdown.\n");
			pthread_exit(0);
		}else{
			printf("msg.recvmsg :%s.\n",msg.recvmsg);
			strcat(buf,"*-*");
			send(tiddata->acceptfd,&msg,sizeof(msg),0);
		}
	}

}

#endif 













