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

#define LOGIN        1
#define REGISTER     2
#define FRIEND_SEE   3
#define FRIEND_ADD   4
#define FRIEND_DEL   5
#define GROUP_SEE    6  
#define GROUP_CREATE 7
#define GROUP_JOIN   8
#define GROUP_QIUT   9
#define GROUP_DEL    10
#define CHAT_ONE     11
#define CHAT_MANY    12
#define FILE_SEND    13
#define FILE_RECV    14
#define EXIT         -1

#define SIZE_PASS_NAME   30
#define MAX_PACK_CONTIAN 100
#define MAX_CHAR         1024

#define DOWNLINE   0
#define ONLINE     1
#define BUZY       2



/**************************************************/

typedef struct  friend_info
{
    int statu;
    int mes_num;
    char name[MAX_CHAR];
}FRIEND_INFO; 




typedef struct user_infor{
    char        username    [MAX_CHAR];
    FRIEND_INFO friends     [MAX_CHAR];
    int         friends_num;
    char        group       [MAX_CHAR][MAX_CHAR];
    int         group_num;
}USER_INFOR;

USER_INFOR m_my_infor;







/******************be sure same with server*******************************/
typedef struct datas{
    char send_name[MAX_CHAR];
    char recv_name[MAX_CHAR];
    int send_fd;
    int recv_fd;
    time_t time;
    char mes[MAX_CHAR*2];
}DATA;

typedef struct package{
    int type;
    DATA  data;
}PACK;



/************************客户端缓冲区**********************/
/*PACK m_pack_send_friend_see [MAX_CHAR];
PACK m_pack_send_chat_one   [MAX_CHAR];
PACK m_pack_send_chat_many  [MAX_CHAR];
PACK m_pack_send_send_file  [MAX_CHAR];


int m_send_num_friend_see;
int m_send_num_chat_one;
int m_send_num_chat_many;
int n_send_num_send_file;*/
/*****************send*********************/
PACK m_pack_send   [MAX_CHAR];
int  m_send_num;


/*****************recv*********************/
PACK m_pack_recv_friend_see   [MAX_PACK_CONTIAN];
PACK m_pack_recv_chat         [MAX_PACK_CONTIAN];
PACK m_pack_recv_send_file    [MAX_PACK_CONTIAN];



int m_recv_num_friend_see;
int m_recv_num_chat;
int m_recv_num_send_file;


/****************************************************/


int m_flag_group_create;
int m_flag_group_join ;
int m_flag_group_del  ;
int m_flag_print_mes;


/***********************function***********************/
void my_err(const char * err_string,int line);
void init();
void sig_close();
int send_login(char username_t [],char password_t []);
int login();
int login_menu();
int send_registe(char username_t[],char password_t[]);
void registe();
int main_menu();


int sockfd;
char *IP = "127.0.0.1";
short PORT = 10222;
typedef struct sockaddr SA;
pthread_mutex_t  mutex_local_user;







void my_err(const char * err_string,int line)
{
	fprintf(stderr, "line:%d  ", line);
	perror(err_string);
	exit(1);
}

void init()
{
	printf("客户端开始启动\n");
    sockfd = socket(AF_INET,SOCK_STREAM,0);//启动socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if(connect(sockfd,(SA*)&addr,sizeof(addr))==-1){
        perror("无法连接到服务器");
        printf("客户端启动失败\n");
        exit(-1);
    }
    printf("客户端启动成功\n");
}



void sig_close(int i)
{
    //关闭客户端的描述符
    close(sockfd);
    exit(0);
}
/*************************tools******************************/

void send_pack(int type,char *send_name,char *recv_name,char *mes)
{
    PACK pack_send_pack;
    time_t timep;
    pack_send_pack.type = type;
    strcpy(pack_send_pack.data.send_name,send_name);
    strcpy(pack_send_pack.data.recv_name,recv_name);
    strcpy(pack_send_pack.data.mes,mes); 
    time(&timep);
    pack_send_pack.data.time = timep;
    if(send(sockfd,&pack_send_pack,sizeof(PACK),0) < 0){
        my_err("send",__LINE__);
    }
}














/*************************login************************************/
int send_login(char username_t[],char password_t[])
{
    PACK recv_login_t;
    int login_judge_flag = 0;
    
    send_pack(LOGIN,username_t,"server",password_t);
    
    if(recv(sockfd,&recv_login_t,sizeof(PACK),0) < 0){
        my_err("recv",__LINE__);
    }
    printf("jijimei????\n");
    login_judge_flag = recv_login_t.data.mes[0] - 48;
    return login_judge_flag;
}


int login()
{
    int flag = 0;
    
    int login_flag = 0;
    char username_t [MAX_CHAR];
    char password_t [MAX_CHAR];
    printf("please input the username:\n");
    scanf("%s",username_t);
    printf("please input the password\n");
    scanf("%s",password_t);

    login_flag = send_login(username_t,password_t);
    if(login_flag ==  2){
        printf("the username is not exit.\n");
        return 0;
    }   
    if(login_flag ==  3 ){
        printf("the user has loged in .\n");
        return 0;
    }  
    if(login_flag == 0) {
        printf("the password is not crrect.\n");
        return 0;
    }
    strcpy(m_my_infor.username,username_t);
    printf("load successfully!\n");
    return 1;
}


int login_menu()
{
    int chioce;
    do
    {
        printf("\n\t\t*******************************\n");
        printf("\t\t*        1.login in           *\n");
        printf("\t\t*        2.register           *\n");
        printf("\t\t*        0.exit              *\n");
        printf("\t\t*******************************\n");
        printf("\t\tchoice：");
        scanf("%d",&chioce);
        switch(chioce)
        {  
            case 1:
                if(login() == 1)
                    return 1;
                break;
            case 2:
                registe();
                break;
            default:
                break;
        }
    }while(chioce!=0);
    return 0;
}
/*******************************************************/







/**************************registe******************************/

int send_registe(char username_t[],char password_t[])
{
    PACK recv_registe_t;
    int send_registe_flag;
    
    send_pack(REGISTER,username_t,"server",password_t);
    
    if(recv(sockfd,&recv_registe_t,sizeof(PACK),0) < 0){
        my_err("recv",__LINE__);
    }
    send_registe_flag = recv_registe_t.data.mes[0] - 48;
    return send_registe_flag;
}




void registe()
{
    int flag = 0;
    flag = REGISTER;
    char username_t[MAX_CHAR];
    char password_t[MAX_CHAR];

   
    
    printf("please input the username you want register:\n");
    scanf("%s",username_t);
    printf("please input the password you want register:\n");
    scanf("%s",password_t);
    if(send_registe(username_t,password_t))
        printf("registe successfully!\n");
    else 
        printf("the name is used ,please input another one\n");
}


/************************************************************/





/*void *clien_send_thread(void *arg)
{
    int i;
    while(1)
    {
        for(i=0;i<m_send_num;i++)
        {
            if(send(sockfd,m_pack_send+i,sizeof(PACK),0) < 0){
               my_err("send",__LINE__);
            }
        }
        sleep(1);
    }
}*/




void change_statu(PACK pack_deal_statu_t)
{
    int count = 0;
    m_my_infor.friends_num=pack_deal_statu_t.data.mes[count++];

    for(int i=1; i <= m_my_infor.friends_num ;i++)
    {
        for(int j=0;j<SIZE_PASS_NAME;j++)
        {
            if(j == 0)   
                m_my_infor.friends[i].statu = pack_deal_statu_t.data.mes[count+j] - 48;
            else
                m_my_infor.friends[i].name[j-1] = pack_deal_statu_t.data.mes[count+j];
        }
        count += SIZE_PASS_NAME;
    }

    m_my_infor.group_num=pack_deal_statu_t.data.mes[count++];
    for(int i=1 ;i <= m_my_infor.group_num ;i++)
    {
        for(int j=0;j<SIZE_PASS_NAME;j++)
        {
            m_my_infor.group[i][j] = pack_deal_statu_t.data.mes[count+j];
        }
        count += SIZE_PASS_NAME;
    }
}




void *deal_statu(void *arg)
{
    int i;
    printf("function deal_statu:\n");
    while(1)
    {
        pthread_mutex_lock(&mutex_local_user); 
        for(i=1;i<=m_recv_num_friend_see;i++)
        {
            //printf("flag1\n");
            change_statu(m_pack_recv_friend_see[i]);
        }
        m_recv_num_friend_see = 0;
        pthread_mutex_unlock(&mutex_local_user);  
    }
}







void *clien_recv_thread(void *arg)
{
    int i;
    PACK pack_t;
    while(1)
    {
        if(recv(sockfd,&pack_t,sizeof(PACK),0) < 0){
            my_err("recv",__LINE__);
        }
        //for(int i=1;i <= )
        pthread_mutex_lock(&mutex_local_user); 
        
        for(int i=1 ;i<= m_my_infor.friends_num;i++)
        {
            if(strcmp(m_my_infor.friends[i].name,pack_t.data.send_name) == 0)
            {
                m_my_infor.friends[i].mes_num++;
                break;
            }
        }

        switch(pack_t.type)
        {
            printf("clien_recv_thread:%d\n", pack_t.type);
            case FRIEND_SEE:
                m_pack_recv_friend_see[++m_recv_num_friend_see] = pack_t;
                break;
            case GROUP_CREATE:
                m_flag_group_create = pack_t.data.mes[0];
                break;
            case GROUP_JOIN:
                m_flag_group_join   = pack_t.data.mes[0];
                break;
            case GROUP_DEL:
                m_flag_group_del    = pack_t.data.mes[0];
            case CHAT_ONE:
                m_pack_recv_chat[++m_recv_num_chat]             = pack_t;
                break;
            case CHAT_MANY:
                m_pack_recv_chat[++m_recv_num_chat]             = pack_t;
                break;
            case SEND_FILE:
                m_pack_recv_send_file[++m_recv_num_send_file]   = pack_t;
                break;
        }
        pthread_mutex_unlock(&mutex_local_user); 
        usleep(1); 
    }
}


void init_clien_pthread()
{
    pthread_t pid_deal_statu,pid_recv;
    pthread_create(&pid_deal_statu,NULL,deal_statu,NULL);
    pthread_create(&pid_recv,NULL,clien_recv_thread,NULL);
} 










/********************friend***************************************/


void get_status_mes()
{
    PACK pack_friend_see;
    pack_friend_see.type = FRIEND_SEE;
    strcpy(pack_friend_see.data.send_name,m_my_infor.username);
    strcpy(pack_friend_see.data.recv_name,"server");
    memset(pack_friend_see.data.mes,0,sizeof(pack_friend_see.data.mes));
    if(send(sockfd,&pack_friend_see,sizeof(PACK),0) < 0){
        my_err("send",__LINE__);
    }
}



void friends_see()
{
   
    pthread_mutex_lock(&mutex_local_user);
    printf("friends num:%d\n", m_my_infor.friends_num); 
    printf("\n\nthe list of friends:\n");
    for(int i=1 ;i<=m_my_infor.friends_num ;i++)
    {
        //printf("m_my_infor.friends[i].statu:%d\n", m_my_infor.friends[i].statu);
        switch(m_my_infor.friends[i].statu)
        {
            case ONLINE:
                printf("ID:%d \033[;32m%s\033[0m[ONLINE] ", i,m_my_infor.friends[i].name);
                if(m_my_infor.friends[i].mes_num)
                    printf("\033[;33m%d messages\033[0m\n", m_my_infor.friends[i].mes_num);
                else 
                    printf("\n");
                break;
            case DOWNLINE:
                printf("ID:%d \033[;31m%s\033[0m[DOWNLINE] ", i,m_my_infor.friends[i].name);
                if(m_my_infor.friends[i].mes_num)
                    printf("\033[;33m%d messages\033[0m\n", m_my_infor.friends[i].mes_num);
                else 
                    printf("\n");
                break;
        }
    }
    pthread_mutex_unlock(&mutex_local_user);  

}


int judge_same_friend(char add_friend_t[])
{
    int i;
    for(i=1;i<=m_my_infor.friends_num;i++)
    {
        if(strcmp(m_my_infor.friends[i].name,add_friend_t) == 0)
            return i;
    }
    return 0;
}



void add_friend()
{
    char add_friend_t[MAX_CHAR];
    
    printf("please input the name of friend you want to add:\n");
    scanf("%s",add_friend_t);

    if(judge_same_friend(add_friend_t))
    {
        printf("you already have same friends!\n");
        return ;
    }
    printf("m_my_infor.username:%s\n", m_my_infor.username);
    send_pack(FRIEND_ADD,m_my_infor.username,"server",add_friend_t);
    get_status_mes();
}


void del_friend()
{
    char del_friend_t[MAX_CHAR];
    printf("please input the name of friend you want to delete:\n");
    scanf("%s",del_friend_t);

    if(!judge_same_friend(del_friend_t))
    {
        printf("you don't have this friends on list!\n");
        return ;
    }
    printf("m_my_infor.username:%s\n", m_my_infor.username);
    
    send_pack(FRIEND_DEL,m_my_infor.username,"server",del_friend_t);
    
    get_status_mes();
}

/*************************end_friend************************************/


















/***********************group***********************************/

int judge_same_group(char *group_name)
{
    int i;
    for(i=1;i<=m_my_infor.group_num;i++)
    {
        if(strcmp(m_my_infor.group[i],group_name) == 0)
            return 1;
    }
    return 0;
}



void group_see()
{
    pthread_mutex_lock(&mutex_local_user); 
    printf("\n\nthe list of group:\n");
    for(int i=1 ;i<=m_my_infor.group_num ;i++)
    {
        printf("ID:%d %s\n", i,m_my_infor.group[i]);
    }
    pthread_mutex_unlock(&mutex_local_user);  
}





void send_mes(char mes_recv_name[],int type)
{
    PACK pack_send_mes;
    char mes[MAX_CHAR];
    time_t timep;
    getchar();
    while(1)
    {   
        time(&timep);
        memset(mes,0,sizeof(mes));
        printf("\n%s",ctime(&timep) );
        if(type == CHAT_ONE)
            printf("%s->",m_my_infor.username);
        fgets(mes,MAX_CHAR,stdin);
        while(mes[0] == 10)
            fgets(mes,MAX_CHAR,stdin);
        if(strcmp(mes,"quit\n") == 0)
            break;
        
        //printf("\t%s\n%s\n", m_my_infor.username,ctime(&timep),mes);

        send_pack(type,m_my_infor.username,mes_recv_name,mes);
    }
    m_flag_print_mes = EXIT;
}



void print_mes(int id)
{
    char group_print_name[MAX_CHAR];
    memset(group_print_name,0,sizeof(group_print_name));
    if(m_pack_recv_chat[id].type == CHAT_ONE)
    {
        printf("\033[40;42m%s\033[0m\t%s\n",m_pack_recv_chat[id].data.send_name,ctime(&m_pack_recv_chat[id].data.time));
        printf("%s\n", m_pack_recv_chat[id].data.mes);
    }
    else
    {
        for(int i=0;i<SIZE_PASS_NAME;i++)
        {
            group_print_name[i] = m_pack_recv_chat[id].data.mes[i];
        }
        printf("\033[40;42m%s\033[0m\t%s\n",group_print_name,ctime(&m_pack_recv_chat[id].data.time));
        printf("%s\n", m_pack_recv_chat[id].data.mes+SIZE_PASS_NAME);
    }
   
}


//less the button to shutdown
void *show_mes(void *username)
{
    int id;
    char *user_name = (char *)username;
    while(1)
    {
        if (m_flag_print_mes == EXIT)
            break;
        pthread_mutex_lock(&mutex_local_user); 
        id = 0;
        for(int i = 1 ;i <= m_recv_num_chat; i++)
        {
            if(strcmp(m_pack_recv_chat[i].data.send_name,user_name) == 0)
            {
                id = i;
                print_mes(id);
                m_recv_num_chat--;
                for(int j = id; j <= m_recv_num_chat&&m_recv_num_chat ;j++)
                {
                    m_pack_recv_chat[j]  =  m_pack_recv_chat[j+1];
                }
                break;
            }
        }
        
        pthread_mutex_unlock(&mutex_local_user); 
        usleep(1);    
    }
}



void send_mes_to_one()
{
    pthread_t pid;
    int id;
    char mes_recv_name[MAX_CHAR];
    friends_see();//print friend list !

    printf("please input the name you want to chat\n");
    scanf("%s",mes_recv_name);
    if (!(id=judge_same_friend(mes_recv_name)))
    {
        printf("sorry,you don't have the friend named !%s\n",mes_recv_name);
        return ;
    }
    m_flag_print_mes = 1;
    m_my_infor.friends[id].mes_num = 0;
    pthread_create(&pid,NULL,show_mes,(void *)mes_recv_name);
    send_mes(mes_recv_name,CHAT_ONE);
}











void group_create()
{
    char group_name[MAX_CHAR];
    printf("please input the group name you want to create:\n");
    scanf("%s",group_name);
    send_pack(GROUP_CREATE,m_my_infor.username,"server",group_name);
    while(!m_flag_group_create);
    printf("m_flag_group_create=%d\n", m_flag_group_create);
    if(m_flag_group_create == 2) 
        printf("create group successfully!\n");
    else if(m_flag_group_create == 1)
        printf("this group has been created!\n");
    m_flag_group_create = 0;
}



void group_join()
{

    char group_name[MAX_CHAR];
    printf("please input the group name you want to join:\n");
    scanf("%s",group_name);
    
    for(int i=1;i <= m_my_infor.group_num ;i++)
    {
        if(strcmp(m_my_infor.group[i],group_name) == 0)
        {
            printf("you have join this group!\n");
            return ;
        }
    }

    send_pack(GROUP_JOIN,m_my_infor.username,"server",group_name);
    while(!m_flag_group_join);
    printf("m_flag_group_join=%d\n", m_flag_group_join);
    if(m_flag_group_join == 2) 
        printf("join group successfully!\n");
    else if(m_flag_group_join == 1)
        printf("there is no group named %s\n",group_name);
    m_flag_group_join = 0;
}


void group_qiut()
{
    char group_name[MAX_CHAR];
    printf("please input the group name you want to qiut:\n");
    scanf("%s",group_name);
    
    for(int i=1;i <= m_my_infor.group_num ;i++)
    {
        if(strcmp(m_my_infor.group[i],group_name) == 0)
        {
            send_pack(GROUP_QIUT,m_my_infor.username,"server",group_name);
            printf("quit group %s successfully!\n",group_name);
            return ;
        }
    }
    
    printf("you did't join this group!\n");
}


void group_del()
{
     char group_name[MAX_CHAR];
    printf("please input the group name you want to delete:\n");
    scanf("%s",group_name);
    for(int i=1;i <= m_my_infor.group_num ;i++)
    {
        if(strcmp(m_my_infor.group[i],group_name) == 0)
        {
            send_pack(GROUP_DEL,m_my_infor.username,"server",group_name);
            while(!m_flag_group_del);
            printf("m_flag_group_del=%d\n", m_flag_group_del);
            if(m_flag_group_del == 2) 
                printf("delete group successfully!\n");
            else if(m_flag_group_del == 1)
                printf("you isn't the owner of group %s\n",group_name);
            return ;
        }
    }
    
    printf("you did't join this group!\n");
}


void send_mes_to_group()
{
    pthread_t pid;
    char mes_recv_group_name[MAX_CHAR];
    group_see();
    printf("please input the group you want to chat\n");
    scanf("%s",mes_recv_group_name);
    if (!judge_same_group(mes_recv_group_name))
    {
        printf("sorry,you don't have the group named !%s\n",mes_recv_group_name);
        return ;
    }

    m_flag_print_mes = 1;
    
    pthread_create(&pid,NULL,show_mes,(void *)mes_recv_group_name);
    send_mes(mes_recv_group_name,CHAT_MANY);
}





void send_file_send(int begin_location,int file_size,char *file_path,char *recv_name)
{
    int fd;
    char mes[MAX_CHAR*2];

    if((fd = open(file_path,O_RDONLY)) == -1)
    {
        my_err("open",__LINE__);
        return ;
    }

    bzero(mes, MAX_CHAR*2); 

    for(int i=0; i < SIZE_PASS_NAME;i++)
    {
        mes[i] = file_path[i];
    }

    // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止 
    while((length = read(fd  ,mes+SIZE_PASS_NAME  ,MAX_CHAR*2 - SIZE_PASS_NAME)) > 0) 
    {
        send_pack(FILE_SEND,m_my_infor.username,recv_name,char *mes)
        bzero(mes, MAX_CHAR*2); 
    } 
    // 关闭文件 
    fclose(fp);

}









void massege()
{

}


int main_menu()
{
    int chioce;
    do
    {
        get_status_mes();
        printf("pack num_chat:%d\n", m_recv_num_chat);
        printf("\n\t\t*******************************\n");
        printf("\t\t*        1.show   friends       *\n");
        printf("\t\t*        2.add    friends       *\n");
        printf("\t\t*        3.delete friends       *\n");
        printf("\t\t*        4.show   group         *\n");
        printf("\t\t*        5.create group         *\n");
        printf("\t\t*        6.join   group         *\n");
        printf("\t\t*        7.quit   group         *\n");
        printf("\t\t*        8.delete group         *\n");
        printf("\t\t*        9.chat with one        *\n");
        printf("\t\t*        10.chat with many      *\n");
        printf("\t\t*        11.send  file          *\n");
        printf("\t\t*        12.message  box        *\n");
        printf("\t\t*        0.exit                 *\n");
        printf("\t\t******************************* *\n");
        printf("\t\tchoice：");
        scanf("%d",&chioce);
        switch(chioce)
        {  
            case 1:
                friends_see();
                break;
            case 2:
                add_friend();
                break;
            case 3:
                del_friend();
                break;
            case 4:
                group_see();
                break;
            case 5:
                group_create();
                break;
            case 6:
                group_join();
                break;
            case 7:
                group_qiut();
                break;
            case 8:
                group_del();
                break;
            case 9:
                send_mes_to_one();
                break;
            case 10:
                send_mes_to_group();
                break;
            case 11:
                send_file();
            default:
                break;
        }
    }while(chioce!=0);
    return 0;
}




int main(int argc, char const *argv[])
{
	int flag =0;

	signal(SIGINT,sig_close);//关闭CTRL+C
    init();//启动并连接服务器

    login_menu();   
    init_clien_pthread();
    main_menu();
  
    

	return 0;
}
