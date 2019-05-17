#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "kvconf.h"
#include<unistd.h>


/**
 * on success, return 0, otherwise return -1
 *
 *  配置文件预处理过程是以一行一行来处理的,大致思路如下:
 *  while(直到文件末尾){
 *      1.删除一行中前面的' ','\t';
 *      2.忽略掉那些以'\n','#','='开头的行;
 *      3.如果一行中有'#'注释,将'#'所在的位置设置'\0',代表字符串末尾;
 *          也就是'#'以及后面注释都不管,因为那是注释  :)
 *      4.删除一行中末尾的换行符;
 *      5.修剪获取到的key,value字符串;
 *      6.剩下的也就是是键值对了,保存在链表中.
 *  } 
 */
int getkvpairs(char* conffile, kvpair** kvpairs){
    /**
     * 如果传入的参数conffile不是NULL,并且配置文件能打开,则使用该文件中的配置参数
     * 如果conffile指定的文件失效,则使用当前文件夹下的./properties.conf文件作为配置
     * 文件,如果前面两者都失效,则会报错,并返回-1,文件后缀conf是properties的缩写
     */
    if(kvpairs == NULL){
        perror("function( getkvpairs ) parameter ( kvpairs ) was NULL\n");
        return -1;
    }

    if (conffile == NULL)
        conffile = "./properties.conf";

    FILE* conf = NULL;
    conf = fopen(conffile, "r");
    if(conf == NULL){
        perror("function( getconfpairs ) can't found the properties file\n");
        return -1; 
    }   

    int     i = 0;                      //用于循环计数
    //int     index = 0;                  //dealWithBuffer数组中作为保存缓存数据的指针
    int     length = 0;                 //保存字符串的长度
    int     equalIndex = 0;             //保存等号的下标
    kvpair* keyValueHead = NULL;        //用于保存键值对的头节点
    kvpair* currentkvpair = NULL;       //用于保存键值对的当前节点
    kvpair* previewkvpair = NULL;       //用于保存键值对的前一个节点
    char*   lineBuffer = calloc(BUFFER_SIZE, sizeof(char));
    char*   dealWithBuffer = calloc(BUFFER_SIZE, sizeof(char));     

    while(fgets(lineBuffer, BUFFER_SIZE, conf)){
       // index = 0;
        equalIndex = 0;
        length = strlen(lineBuffer);
        /**
         * 删除行首的空格,制表符
         */
        for(i = 0; i < length; i++){ 
            if((lineBuffer[i] != ' ') && (lineBuffer[i] != '\t')){
                strcpy(dealWithBuffer, &(lineBuffer[i]));
                break;
            }
        }
        /**
         *  清除一行中有#来注释的部分,保留键值对
         *  且找出一行中=所在的位置,位置信息保存在equalIndex中
         */
        length = strlen(dealWithBuffer);
        for(i = 0; i < length; i++){ 
            if(dealWithBuffer[i] == '#' ){
                dealWithBuffer[i++] = '\0';  
                break;
            }else if(dealWithBuffer[i] == '=' ){
                equalIndex = i;
            }
        }
        /**
         * 删除以换行符,#,=等字符开始的行,同时清空dealWithBuffer缓冲区
         */
        if((equalIndex == 0) || (lineBuffer[ 0 ] == '\n') || (lineBuffer[ 0 ] == '#')) {
            /**
             * 一定要记得清理这个缓存
             */
            cleanString(dealWithBuffer);
            continue;
        }
        /**
         * 如果一行数据末尾是'\n',则换成'\0',相当于移除'\n'
         */
        length = strlen(dealWithBuffer);
        if(dealWithBuffer[length-1] == '\n'){
            dealWithBuffer[length-1] = '\0';
        }
        /**
         * 通过将'='换成'\0',这样就key,value字符串
         */
        dealWithBuffer[equalIndex] = '\0';
        /**
         * 一定要的得加1, 因为字符串长度不包括尾零
         */
        char* key = calloc(strlen(dealWithBuffer)+1, sizeof(char));
        char* value = calloc(strlen(&(dealWithBuffer[equalIndex+1]))+1, sizeof(char));
        strcpy(key, dealWithBuffer);
        strcpy(value, &(dealWithBuffer[equalIndex+1]));

        /**
         * 修剪key,value的值,也就是去掉字符串左右两边的' ','\t'
         */
        trim(key);
        trim(value);
        /**
         * 接下来检查key是否存在,如果存在,直接修改其value,而不创建数据结构体
         * 如果key不存在,则创建结构体,保存key,value,加入链表
         * 当然,先要保证key,value有效
         */
        if((strlen(key) != 0) && (strlen(value) != 0)){
            if((currentkvpair = checkKey(key, keyValueHead)) != NULL){
                bzero(currentkvpair->value, strlen(currentkvpair->value));
                strcpy(currentkvpair->value, value);
            }else{
                currentkvpair = malloc(sizeof(kvpair));
                strcpy(currentkvpair->key, key);
                strcpy(currentkvpair->value, value);
                currentkvpair->next = NULL;
                if(keyValueHead == NULL){ 
                    keyValueHead = currentkvpair;
                    previewkvpair = currentkvpair;
                }else {
                    previewkvpair->next =  currentkvpair;
                    previewkvpair =  currentkvpair;
                    currentkvpair = NULL;
                }
            }
        }
        bzero(dealWithBuffer, BUFFER_SIZE);//不能使用cleanString清理,因为字符中间有'\0'
        cleanString(lineBuffer);
        free(key);
        free(value);
    }
    free(lineBuffer);
    free(dealWithBuffer);
    *kvpairs = keyValueHead;
    fclose(conf);
    return 0;
}

void cleanString(char* string){
    int i;
    int length = strlen(string);
    for(i = 0; i < length; i++){
        string[i] = '\0';
    }
}

char* key2val(char* key, kvpair* kvpairs){
    if((key == NULL) || (strlen(key) == 0)){
        perror("function( key2val) parameter ( key ) was NULL\n");
        return NULL;
    }
    
    kvpair* currentkvpair = kvpairs;
    while(currentkvpair){
        /**
          * 本来打算直接用strcmp,但是貌似strcmp会自动比较字符串所占数组的大小
          * 所以改成使用strncmp
          */
        if(strncmp(currentkvpair->key, key, strlen(key)) == 0){
            return currentkvpair->value;
        }
        currentkvpair = currentkvpair->next;
    }
    return NULL;
}

char* val2key(char* value, kvpair* kvpairs){
    if((value == NULL) || (strlen(value) == 0)){
        perror("function( val2key) parameter ( value ) was NULL\n");
        return NULL;
    }

    kvpair* currentkvpair = kvpairs;
    while(currentkvpair){
        if(strncmp(currentkvpair->value, value, strlen(value)) == 0){
            return currentkvpair->key;
        }
        currentkvpair = currentkvpair->next;
    }
    return NULL;
}

kvpair* checkKey(char* key, kvpair* kvpairs){
    if((key == NULL) || (strlen(key) == 0)){
        perror("function( checkKey ) parameter ( key ) was NULL\n");
        return NULL;
    }
    
    kvpair* currentkvpair = kvpairs;
    while(currentkvpair){
        if(strncmp(currentkvpair->key, key, strlen(key)) == 0){
            return currentkvpair;
        }
        currentkvpair = currentkvpair->next;
    }
    return NULL;
}

void printkvpairs(kvpair* kvpairs){
    if(kvpairs == NULL){
        perror("function( printkvpairs ) parameter( kvpairs ) was NULL\n");
        return;
    }

    int index = 1;
    kvpair* currentkvpair = kvpairs;
    printf("\033[32m--------------------------------------\033[0m\n");
    while(currentkvpair){
        printf("\033[32m   %03d: %s=%s\033[0m\n", index, currentkvpair->key, currentkvpair->value);
        currentkvpair = currentkvpair->next;
        index++;
    }
    printf("\033[32m--------------------------------------\033[0m\n");
}

int freekvpairs(kvpair* kvpairs){
    if(kvpairs == NULL){
        return 0;
    }

    kvpair* previewkvpair = kvpairs;
    kvpair* currentkvpair = kvpairs;
    while(currentkvpair->next){
        previewkvpair = currentkvpair;
        currentkvpair = currentkvpair->next;
        free(previewkvpair);
    }
    free(currentkvpair);
    return 0;
}

char *ltrim(char* str) {
    char str_tmp[BUFFER_SIZE] = {0};
    char *current = str_tmp;
    //int count = 0;

    strncpy(str_tmp, str, strlen(str));
    bzero(str, strlen(str));

    while(' ' == (*current) || ('\t' == *current ))
        current++;

    strncpy(str, current, strlen(current));
    return str;
}

char *rtrim(char* str) {
    //int count = 0;
    int i = strlen(str)-1;
    for(; i >= 0; i--){
        if((' ' == str[i]) || ('\t' == str[i]) || ('\0' == str[i]))
            str[i] = '\0';
        else 
            break;
    }
    return str;
}

char *trim(char* str) {
    return rtrim(ltrim(str));
}

void testParseConf(int argc, char* argv[]){
    //传入需要被解析的文件
    if(argc < 2){
        printf("    Usage:\n\r        ./parseConf <configure file> \n\n");
        return ;
    }

    /**
     * 获取键值对,键值对头节点保存在keyValues中
     */
    kvpair* keyValues;
    getkvpairs(argv[1], &keyValues);
    printf("\n\033[32m\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\033[36mDemo\033[32m/////////////////\033[0m\n");

    /**
     * 将配置文件中的内容打印出来
     */
    int fd = open(argv[1], O_RDONLY);
    if(-1 == fd){
        perror("open file error");
    }

    char buffer[1024] = {0};
    read(fd, buffer, 1024);
    printf("%s\n", buffer);

    close(fd);

    /**
     * 将当前的所有的键值对打印出来
     */
    printkvpairs(keyValues);
    /**
     * 通过key获取value值
     */
    char* key = "ip";
    printf("\033[32mgetValueBykey:key = %s; value = %s\033[0m\n", key, key2val(key, keyValues));
    /**
     * 通过value获取key值
     */
    char* value = "22";
    printf("\033[32mgetKeyByValue:value = %s; key = %s\033[0m\n", value, val2key(value, keyValues));
    printf("\033[32m--------------------------------------\033[0m\n");
    /**
     * 释放keyValues链表
     */
    if(freekvpairs(keyValues) == 0){
        printf("\033[32m Memory of keyValues linked has freed\033[0m\n");
        printf("\033[32m--------------------------------------\033[0m\n");
    }
}
/*
int main(int argc, char **argv) {
    testParseConf(argc, argv);
}*/