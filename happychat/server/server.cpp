#include <mysql/mysql.h> 
#include <gtk/gtk.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define LOGIN      1
#define REGISTER   2
#define FRIEND_SEE 3
#define FRIEND_ADD 4
#define FRIEND_DEL 5
#define CHAT_ONE   6
#define CHAT_MANY  7
#define SEND_FILE  8
#define EXIT      -1

#define MAX_CHAR  1024
#define EPOLL_MAX 200000
#define LISTENMAX 1000
#define USER_MAX  100  //the user on line
#define GROUP_MAX 50

#define DOWNLINE   0
#define ONLINE     1
#define BUZY       2


/********************user infor***********************************/
typedef struct infor_user 
{
    char username[MAX_CHAR];
    char password[MAX_CHAR];
    int  statu;//don't foget to change is to 0 when the server begin
    int  socket_id;
    char friends[MAX_CHAR][MAX_CHAR];
    int  friends_num;
    char groups[MAX_CHAR][MAX_CHAR];
    char groups_num;
}INFO_USER;


INFO_USER  m_infor_user  [USER_MAX];
int        m_user_num; 
/**************************************************/



/*********************group infor***************************/
typedef struct infor_group
{
    int   user_num;
    char  user_name[MAX_CHAR][MAX_CHAR];
}INFO_GROUP;

INFO_GROUP   m_infor_group  [GROUP_MAX];
int          m_group_num;
/*********************************************************/



typedef struct datas{
    char     send_name[MAX_CHAR];
    char     recv_name[MAX_CHAR];
    int      send_fd;
    int      recv_fd;
    time_t   time;
    char     mes[MAX_CHAR*2];
}DATA;

typedef struct package{
    int   type;
    DATA  data;
}PACK;


/**********function***************/
void my_err(const char * err_string,int line);
void signal_close(int signal_num);
int find_userinfor(char username_t[]);
void login(PACK *recv_pack);
void registe(PACK *recv_pack);
void *deal(PACK *recv_pack);
void *serv_send_thread(void *arg);
void init_server_pthread();
int judge_usename_same(char username_t[]);





int listenfd,epollfd;//
char* IP = "127.0.0.1";//服务器IP
short PORT = 10222;//端口号
typedef struct sockaddr SA;//类型转换
pthread_mutex_t  mutex;



/*****************send*********************/
PACK m_pack_send   [MAX_CHAR*2];
int  m_send_num;


/**************************************/



void my_err(const char * err_string,int line)
{
	fprintf(stderr, "line:%d  ", line);
	perror(err_string);
	exit(1);
}




void signal_close(int i)
{
//关闭服务器前 关闭服务器的socket
    //close(listenfd);
    printf("服务器已经关闭\n");
    exit(1);
}





/**********************login*******************************************/

/*void set_user_online(char username_t[])
{
    int i;
    for(i=1;i<=m_send_num;i++)
    {
        if(strcmp(m_infor_user[i].username,username_t) == 0)
        {
            m_infor_user[i].statu = ONLINE;
            return ;
        }
    }
}*/





int find_userinfor(char username_t[])
{

    int i;
    if(m_user_num == 0)  return 0;
    for(i=1;i<=m_user_num;i++)
    {
        if(strcmp(m_infor_user[i].username,username_t) == 0)
            return i;
    }
    if(i == m_user_num+1) 
        return 0;
}




void login(PACK *recv_pack)
{
    int id=0;
    char login_flag[10];

    if((id=find_userinfor(recv_pack->data.send_name)) == 0){//not exit username
        login_flag[0] = '2';
    }
    else if(strcmp(recv_pack->data.mes,m_infor_user[id].password) == 0){//the password is not crrect
        login_flag[0] = '1';
        //change user infor
        m_infor_user[id].statu = ONLINE;
        m_infor_user[id].socket_id = recv_pack->data.send_fd; 
        printf("%s get online!\n", m_infor_user[id].username);

    }
    else login_flag[0] = '0';
    
    login_flag[1] = '\0';
    printf("%s\n", recv_pack->data.send_name);
    printf("%s\n", recv_pack->data.mes);
    printf("%d\n", m_user_num);     
    strcpy(recv_pack->data.recv_name,recv_pack->data.send_name);
    strcpy(recv_pack->data.send_name,"server");
    strcpy(recv_pack->data.mes,login_flag);
    recv_pack->data.recv_fd = recv_pack->data.send_fd;
    recv_pack->data.send_fd = listenfd;
    
    pthread_mutex_lock(&mutex);  
    memcpy(&m_pack_send[m_send_num++],recv_pack,sizeof(PACK));
    pthread_mutex_unlock(&mutex);  
    
    free(recv_pack);
}
/**********************************************************/




/********************registe********************************/
int judge_usename_same(char username_t[])
{
    int i;
    if(m_user_num == 0)  return 1;
    for(i=1;i<=m_user_num;i++)
    {
        if(strcmp(m_infor_user[i].username,username_t) == 0)
            return 0;
    }
    if(i == m_user_num+1) 
        return 1;
}




void registe(PACK *recv_pack)
{
    char registe_flag[10];
 
    
    if(judge_usename_same(recv_pack->data.send_name)){//the password is not crrect
        registe_flag[0] = '1';
        //add user infor
        m_user_num++;
        strcpy(m_infor_user[m_user_num].username,recv_pack->data.send_name);
        strcpy(m_infor_user[m_user_num].password,recv_pack->data.mes);
        printf("\nfunction registe:\n");
        printf("*registe success!\n");
        printf("*username:%s\n", m_infor_user[m_user_num].username);
        printf("*password:%s\n", m_infor_user[m_user_num].password);
        printf("%d\n", m_user_num);

        m_infor_user[m_user_num].statu = DOWNLINE;
    }
    else registe_flag[0] = '0';
    registe_flag[1] = '\0';
   
    strcpy(recv_pack->data.recv_name,recv_pack->data.send_name);
    strcpy(recv_pack->data.send_name,"server");
    strcpy(recv_pack->data.mes,registe_flag);
    recv_pack->data.recv_fd = recv_pack->data.send_fd;
    recv_pack->data.send_fd = listenfd;
    
    pthread_mutex_lock(&mutex);  
    memcpy(&m_pack_send[m_send_num++],recv_pack,sizeof(PACK));
    pthread_mutex_unlock(&mutex);  
   
    free(recv_pack);
}
/**********************************************************/






void *deal(void *recv_pack_t)
{
    int i;
    PACK *recv_pack = (PACK *)recv_pack_t;
    printf("\n\n\ndeal function = %d\n", recv_pack->type);
  
    switch(recv_pack->type)
    {
        case LOGIN:
            login(recv_pack);
            break;
        case REGISTER:
            registe(recv_pack);
            break;
        case FRIEND_SEE:

            break;
        case FRIEND_ADD:

            break;
        case FRIEND_DEL:

            break;
        case CHAT_ONE:

            break;
        case CHAT_MANY:

            break;
        case SEND_FILE:

            break;
        case EXIT:

            break;       
    }

}





/************************send_thread*****************************/
void *serv_send_thread(void *arg)
{
    int i,recv_fd_t;
    while(1)
    {
        pthread_mutex_lock(&mutex); 
        //printf("serv_send_thread:%d\n", m_send_num);
        for(i = m_send_num-1;i>=0;i--)
        {
            recv_fd_t = m_pack_send[i].data.recv_fd;
            if(send(recv_fd_t,&m_pack_send[i],sizeof(PACK),0) < 0){
                my_err("send",__LINE__);
            }
            printf("serv_send_thread:%d\n", m_send_num);
        }
        m_send_num = 0;
       pthread_mutex_unlock(&mutex);  
    }
}


void init_server_pthread()
{
    printf("\ninit_server_pthread\n");
    pthread_t pid_send;
    pthread_create(&pid_send,NULL,serv_send_thread,NULL);
} 

/****************************************************/





int main()
{
    int n;
    pthread_t pid;
    PACK recv_t;
    PACK *recv_pack;
    int err,socketfd,fd_num;
    struct sockaddr_in fromaddr;
    socklen_t len = sizeof(fromaddr);
    struct epoll_event ev, events[LISTENMAX];


    signal(SIGINT,signal_close);//退出CTRL+C
    pthread_mutex_init(&mutex, NULL);  


    printf("服务器开始启动..\n");
    init_server_pthread();
    epollfd = epoll_create(EPOLL_MAX);//生成epoll句柄
    
    listenfd = socket(AF_INET,SOCK_STREAM,0);//启动socket
    if(listenfd == -1){
        perror("创建socket失败");
        printf("服务器启动失败\n");
        exit(-1);
    }

    err = 1;

    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&err,sizeof(int)); 

    ev.data.fd = listenfd;//设置与要处理事件相关的文件描述符
    ev.events = EPOLLIN;//设置要处理的事件类型
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);//注册epoll事件

    //准备网络通信地址
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if(bind(listenfd,(SA*)&addr,sizeof(addr))==-1){//绑定服务器
        perror("绑定失败");
        printf("服务器启动失败\n");
        exit(-1);
    }
    printf("成功绑定\n");

    //设置监听
    if(listen(listenfd,10)==-1){
        perror("设置监听失败");
        printf("服务器启动失败\n");
        exit(-1);
    }
    printf("设置监听成功\n");
    printf("初始化服务器成功\n");
    printf("服务器开始服务\n");
    while(1)
    {

        fd_num = epoll_wait(epollfd, events, EPOLL_MAX, 1000);
        for(int i=0; i<fd_num; i++)
        {
            if(events[i].data.fd == listenfd)
            {   

                socketfd = accept(listenfd,(SA*)&fromaddr,&len);
                printf("%d get conect!\n",socketfd);
                ev.data.fd = socketfd;
                ev.events = EPOLLIN;//设置监听事件可写
                //新增套接字
                epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &ev);
            }
            else if(events[i].events & EPOLLIN)//fd can read
            {

                n = recv(events[i].data.fd,&recv_t,sizeof(PACK),0);//读取数据
                recv_t.data.send_fd = events[i].data.fd;
                printf("\n\n\n***PACK***\n");
                printf("*send_name: %s\n", recv_t.data.send_name);
                printf("*recv_name: %s\n",recv_t.data.recv_name);
                printf("*mes      : %s\n", recv_t.data.mes);
                printf("send_fd   : %d\n", recv_t.data.send_fd);
                printf("recv_fd   : %d\n", recv_t.data.recv_fd);
                printf("********\n\n");
                if(n < 0)//recv错误
                {     
                    close(events[i].data.fd);
                    perror("recv");
                    continue;
                }
                else if(n == 0)
                {
                    for(int i=1;i<=m_user_num;i++)
                    {
                        if(events[i].data.fd == m_infor_user[i].socket_id)
                        {
                            printf("%s down line!",m_infor_user[i].username);
                            m_infor_user[i].statu = DOWNLINE;
                        }
                    }   
                    ev.data.fd = events[i].data.fd;
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, &ev);//删除套接字
                    close(events[i].data.fd);
                    continue;   
                }
                recv_pack = (PACK*)malloc(sizeof(PACK));
                memcpy(recv_pack, &recv_t, sizeof(PACK));
                pthread_create(&pid,NULL,deal,(void *)recv_pack);
            }

        }
    }
    return 0;
}
