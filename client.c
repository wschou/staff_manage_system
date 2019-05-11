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

#include "common.h"

/**************************************
 *函数名：do_query
 *参   数：消息结构体
 *功   能：登陆
 ****************************************/
void do_admin_query(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);



}


/**************************************
 *函数名：admin_modification
 *参   数：消息结构体
 *功   能：管理员修改
 ****************************************/
void do_admin_modification(int sockfd,MSG *msg)//管理员修改
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);


}


/**************************************
 *函数名：admin_adduser
 *参   数：消息结构体
 *功   能：管理员创建用户
 ****************************************/
void do_admin_adduser(int sockfd,MSG *msg)//管理员添加用户
{		
	time_t now;
	struct tm *tm_now;
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	printf("测试添加新用户\n");
	memset(&msg->info, 0, sizeof(staff_info_t));
	
	msg->info.no = 1005;
	msg->info.usertype = USER;
	strcpy(msg->info.name, "wanger");
	strcpy(msg->info.passwd, "121121");
	msg->info.age = 17;
	strcpy(msg->info.phone, "13512345678");
	strcpy(msg->info.addr, "北京市海淀区悦秀路83号");
	strcpy(msg->info.work, "码农");
	//strcpy(msg->info.date, "2019.05.11");
	time(&now);
	tm_now = localtime(&now);
	sprintf(msg->info.date, "%d-%d-%d",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday);
	msg->info.level = 0;
	msg->info.salary = 7998.02;
	printf("员工信息结构体填充完成\n");
#if 0
	time(&now);
	//printf("The number of seconds since January 1, 1970 is  %ld\n",now);
	tm_now = localtime(&now);
	//printf("now datetime: %d-%d-%d %d:%d:%d\n",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
	/*日期*/
	sprintf(msg->info.date, "%d-%d-%d\n",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday);
	printf("date: %s\n", msg->info.date);
	/*时间*/
	sprintf(msg->info.date, "%d:%d:%d\n",tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
	printf("time: %s\n", msg->info.date);
	/*日期 时间*/
	sprintf(msg->info.date, "%d-%d-%d %d:%d:%d\n",tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
	printf("datetime: %s\n", msg->info.date);
#endif
	msg->msgtype  = ADMIN_ADDUSER;
	msg->usertype = ADMIN;
	strcpy(msg->recvmsg, "Client: add user");
	//发送添加请求
	send(sockfd, msg, sizeof(MSG), 0);
	//接受服务器响应
	recv(sockfd, msg, sizeof(MSG), 0);
	printf("msg->recvmsg :%s\n",msg->recvmsg);
}


/**************************************
 *函数名：admin_deluser
 *参   数：消息结构体
 *功   能：管理员删除用户
 ****************************************/
void do_admin_deluser(int sockfd,MSG *msg)//管理员删除用户
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);

}



/**************************************
 *函数名：do_history
 *参   数：消息结构体
 *功   能：查看历史记录
 ****************************************/
void do_admin_history (int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);

}


/**************************************
 *函数名：admin_menu
 *参   数：套接字、消息结构体
 *功   能：管理员菜单
 ****************************************/
void admin_menu(int sockfd,MSG *msg)
{
	int n;

	while(1)
	{
		printf("*************************************************************\n");
		printf("* 1：查询  2：修改 3：添加用户  4：删除用户  5：查询历史记录*\n");
		printf("* 6：退出													*\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
		case 1:
			msg->msgtype = ADMIN_QUERY;
			send(sockfd, msg, sizeof(MSG), 0);
			do_admin_query(sockfd,msg);
			break;
		case 2:
			msg->msgtype = ADMIN_MODIFY;
			send(sockfd, msg, sizeof(MSG), 0);
			do_admin_modification(sockfd,msg);
			break;
		case 3:
			msg->msgtype = ADMIN_ADDUSER;
			send(sockfd, msg, sizeof(MSG), 0);
			do_admin_adduser(sockfd,msg);
			break;
		case 4:
			msg->msgtype = ADMIN_DELUSER;
			send(sockfd, msg, sizeof(MSG), 0);
			do_admin_deluser(sockfd,msg);
			break;
		case 5:
			msg->msgtype = ADMIN_HISTORY;
			send(sockfd, msg, sizeof(MSG), 0);
			do_admin_history(sockfd,msg);
			break;
		case 6:
			msg->msgtype = QUIT;
			send(sockfd, msg, sizeof(MSG), 0);
			close(sockfd);
			exit(0);
		default:
			printf("您输入有误，请重新输入！\n");
		}
	}
}






/**************************************
 *函数名：do_query
 *参   数：消息结构体
 *功   能：登陆
 ****************************************/
void do_user_query(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);

}



/**************************************
 *函数名：do_modification
 *参   数：消息结构体
 *功   能：修改
 ****************************************/
void do_user_modification(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);

}


/**************************************
 *函数名：user_menu
 *参   数：消息结构体
 *功   能：管理员菜单
 ****************************************/
void user_menu(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int n;
	while(1)
	{
		printf("*************************************************************\n");
		printf("*************  1：查询  	2：修改		3：退出	 *************\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
		case 1:
			do_user_query(sockfd,msg);
			break;
		case 2:
			do_user_modification(sockfd,msg);
			break;
		case 3:
			msg->msgtype = QUIT;
			send(sockfd, msg, sizeof(MSG), 0);
			close(sockfd);
			exit(0);
		default:
			printf("您输入有误，请输入数字\n");
			break;
		}
	}
}




int admin_or_user_login(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//输入用户名和密码
	memset(msg->username, 0, NAMELEN);
	printf("请输入用户名：");
	scanf("%s",msg->username);
	getchar();

	memset(msg->passwd, 0, DATALEN);
	printf("请输入密码（6位）");
	scanf("%s",msg->passwd);
	getchar();

	//发送登陆请求
	send(sockfd, msg, sizeof(MSG), 0);
	//接受服务器响应
	recv(sockfd, msg, sizeof(MSG), 0);
	printf("msg->recvmsg :%s\n",msg->recvmsg);

	//判断是否登陆成功
	if(strncmp(msg->recvmsg, "OK", 2) == 0)
	{
		if(msg->usertype == ADMIN)
		{
			printf("亲爱的管理员，欢迎您登陆员工管理系统！\n");
			admin_menu(sockfd,msg);
		}
		else if(msg->usertype == USER)
		{
			printf("亲爱的用户，欢迎您登陆员工管理系统！\n");
			user_menu(sockfd,msg);
		}
	}
	else
	{
		printf("登陆失败！%s\n", msg->recvmsg);
		return -1;
	}

	return 0;
}


/************************************************
 *函数名：do_login
 *参   数：套接字、消息结构体
 *返回值：是否登陆成功
 *功   能：登陆
 *************************************************/
int do_login(int sockfd)
{	
	int n;
	MSG msg;

	while(1){
		printf("*************************************************************\n");
		printf("********  1：管理员模式    2：普通用户模式    3：退出********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
		case 1:
			msg.msgtype  = ADMIN_LOGIN;
			msg.usertype = ADMIN;
			break;
		case 2:
			msg.msgtype =  USER_LOGIN;
			msg.usertype = USER;
			break;
		case 3:
			msg.msgtype = QUIT;
			if(send(sockfd, &msg, sizeof(MSG), 0)<0)
			{
				perror("do_login send");
				return -1;
			}
			close(sockfd);
			exit(0);
		default:
			printf("您的输入有误，请重新输入\n"); 
		}

		admin_or_user_login(sockfd,&msg);
	}

}


int main(int argc, const char *argv[])
{
	//socket->填充->绑定->监听->等待连接->数据交互->关闭 
	int sockfd;
	int acceptfd;
	ssize_t recvbytes,sendbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	socklen_t cli_len = sizeof(clientaddr);

	//创建网络通信的套接字
	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("socket failed.\n");
		exit(-1);
	}
	printf("sockfd :%d.\n",sockfd); 

	//填充网络结构体
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	//	serveraddr.sin_port   = htons(atoi(argv[2]));
	//	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port   = htons(5001);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(sockfd,(const struct sockaddr *)&serveraddr,addrlen) == -1){
		perror("connect failed.\n");
		exit(-1);
	}

	do_login(sockfd);

	close(sockfd);

	return 0;
}



