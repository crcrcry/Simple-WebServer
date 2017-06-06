#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#define PORT 3431

struct sockaddr_in s_addr;  //服务器网络地址
unsigned int sin_size;  //服务器网络地址长度
int sfp; //服务器socket标志符

//客户端结构
struct client{
    int id;
    int fd;   //socket标志符
    struct sockaddr_in c_addr;  //客户端socket网络地址
    pthread_t process; //处理子线程
}c[100]; //所有客户端结构数组

//数字转化为字符串
void inttochar(int x, char str[]){
    int i, count, result = 1;
    for(i = 0; (int)result > 0; i++){
        result = (x / pow(10, i));
    }
    count = i - 1;
    for(i = 0; i < count; i++){
        int plus = (int)pow(10, (count-1-i));
        str[i] = (x / plus) + '0';
        x = x%plus;
    }
    str[count] = '\0';
}


void* response(void* arg){
    int id = *(int *)arg;
    char req[65536], res[65536];
    
    FILE *fp;       //文件标志符
    char* method;   //get、post
    char url[256], url_tmp[64], type[16];    //url：绝对路径  url_tmp：根目录路径   type：文件类型
    char content[65536];    //文件内容
    int length;         //文件长度
    char length_str[64];    //转化为字符串
    
    while (1) {
        if(recv(c[id].fd, req, 65536, 0)){
            puts(req);
            strcpy(res, "");
            
            //处理method
            method = strstr(req, "GET");
            
            //处理get请求
            if(method){
                //url: directory转到directory/index.html
                strncpy(url_tmp, strchr(req, '/')+1, (strstr(req, "HTTP")-strchr(req, '/')-2));
                url_tmp[strstr(req, "HTTP")-strchr(req, '/')-2] = '\0';
                if(strchr(url_tmp, '.') == NULL){
                    strcat(url_tmp, "index.html");
                }
                //此处为我的电脑上部署的绝对路径，部署服务器时需要修改下面的路径
                strcpy(url, "/Users/crcrcry/Documents/Git/Network_Exp/Network-Exp8/Simple_WebServer/Server_Root/");
                strcat(url, url_tmp);
                //type
                strcpy(type, strchr(url_tmp, '.')+1);
                
                printf("URL: %s\n",url);
                printf("URL_tmp: %s\n",url_tmp);
                printf("type: %s\n\n",type);
                
                //打开文件
                fp = fopen(url, "rb+");
                
                if(fp == NULL){
                    //404 not found
                    strcpy(res, "HTTP/1.1 404 Not Found\nContent-Type: text/html;charset=ISO-8859-1\nContent-Length: 300\n\n<html><body><h1>404 Not Found</h1><p>Cr's server: URL is wrong.</p></body></html>\n\n");
                }else{
                    //连接http响应包文本
                    length = (int)fread(content, 1, 65536, fp);
                    puts(content);
                    inttochar(length, length_str);
                    
                    //对html和txt类型处理
                    if(strcmp(type, "html")==0||strcmp(type, "txt")==0){
                        strcat(res, "HTTP/1.1 200 OK\nContent-Type: ");
                        strcat(res, "text/");
                        strcat(res, type);
                        strcat(res, ";charset=ISO-8859-1\n");
                    }
                    //对jpg和png类型的处理
                    else if(strcmp(type, "jpg")==0||strcmp(type, "png")==0){
                        strcat(res, "HTTP/1.1 200 OK\nCache-Control: no-cache\nContent-Type: ");
                        strcat(res, "image/");
                        if(strcmp(type, "jpg")==0){
                            strcpy(type, "jpeg");
                        }
                        strcat(res, type);
                        strcat(res, "\nAccept-Ranges: bytes\n");
                    }
                    //响应包正文长度
                    strcat(res, "Content-Length: ");
                    strcat(res, length_str);
                    strcat(res, "\n\n");
                    
                    //响应包正文，注意因为图片中有结束符，所以不能用strcat
                    int i;
                    int now_length = (int)strlen(res);
                    for(i = now_length; i < now_length+length; i++){
                        res[i] = content[i - now_length];
                    }
                    res[i] = '\n';
                    res[i+1] = '\n';
                    res[i+2] = '\0';
                }
            }
            //处理post请求
            else{
                strncpy(url_tmp, strchr(req, '/')+1, (strstr(req, "HTTP")-strchr(req, '/')-2));
                url_tmp[strstr(req, "HTTP")-strchr(req, '/')-2] = '\0';
                if(strcmp(url_tmp, "html/dopost") == 0){
                    //200
                    char login[20], pwd[20];
                    char* info;
                    
                    //用户名密码数据获取
                    info = strstr(req, "login=");
                    strcpy(login, info+6);
                    login[strstr(info, "&")-info-6] = '\0';
                    strcpy(pwd, strstr(info, "&")+6);
                    
                    //密码正确
                    if(strcmp(login, "3140103431")==0 && strcmp(pwd, "3431") == 0){
                        strcpy(res, "HTTP/1.1 200 ok\nContent-Type: text/html;charset=utf-8\nContent-Length: 300\n\n<html><body><h1>3140103431 登陆成功</h1></body></html>\n\n");
                    }
                    //密码错误
                    else{
                        strcpy(res, "HTTP/1.1 200 ok\nContent-Type: text/html;charset=utf-8\nContent-Length: 300\n\n<html><body><h1>登陆失败</h1></body><script>alert(\"用户名或密码错误\");</script></html>\n\n");
                    }
                    
                }else{
                    //404 not found
                    strcpy(res, "HTTP/1.1 404 Not Found\nContent-Type: text/html;charset=ISO-8859-1\nContent-Length: 300\n\n<html><body><h1>404 Not Found</h1><p>Cr's server: post URL is wrong.</p></body></html>\n\n");
                }
            }
            
            send(c[id].fd, res, 65536, 0);
        }
    }
    
    return NULL;
}

int main(){
    int count = 0;
    //创建一个socket
    sfp = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sfp)
    {
        printf("socket fail ! \r\n");
        return -1;
    }
    
    //创建服务器ip、port等网络数据
    bzero(&s_addr,sizeof(struct sockaddr_in));
    s_addr.sin_family=AF_INET;
    s_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    s_addr.sin_port=htons(PORT);
    
    //将服务器网络数据与socket绑定
    bind(sfp,(struct sockaddr *)(&s_addr), sizeof(struct sockaddr));
    
    //将socket加入监听队列
    if(-1 == listen(sfp,5))
    {
        printf("listen fail !\r\n");
        return -1;
    }
    
    sin_size = sizeof(struct sockaddr_in);
    printf("Server start, Socket start.\n");
    
    //循环等待客户端链接
    while (1) {
        int id;
        //accept阻塞，等待客户端连接
        c[count].fd = accept(sfp, (struct sockaddr *)(&c[count].c_addr), &sin_size);
        
        if(-1 == c[count].fd)
        {
            printf("accept fail !\r\n");
            return -1;
        }
        id = count;
        //创建客户端发送、接受处理线程
        pthread_create(&c[count].process, NULL, response, &id);
        
        count++;
    }
    
}
