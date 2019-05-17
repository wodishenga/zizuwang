//默认一行数据的缓冲区大小
#define BUFFER_SIZE 1024

//键值对结构体,本Demo采用单链表来实现
typedef struct KVPAIR {
    char key[128];
    char value[512];
    struct KVPAIR * next;
} kvpair;
/**
 *  获取键值对的起始指针,参数是传入需要保存keyvalus首地址的指针,
 *  函数返回值为0时表示获取成功
 */
int getkvpairs(char *conffile, kvpair** kvpairs);
/**
 *  通过key值获取kvpairs中的value,如果链表中没有key对应的数据,或者给的参数错误
 *  将返回NULL
 */
char* key2val(char* key, kvpair* kvpairs);
/**
 *  通过value值获取kvpairs中的key,如果链表中没有value对应的数据,或者给的参数错误
 *  将返回NULL
 */
char* val2key(char* value, kvpair* kvpairs);
//打印输出kvpairs中所有的键值对
void printkvpairs(kvpair* kvpairs);
//用'\0'填充字符串
void cleanString(char* string);
/**
 *  查看链表中有没有当前key对应的键值对,如果有,返回该key对应的键值对
 *  如果没有,将返回NULL
 */
kvpair* checkKey(char* key, kvpair* kvpairs);
//释放链表
int freekvpairs(kvpair* kvpairs);
//去除字符串左侧不可见字符
char *ltrim(char* str);
//去除字符串右侧不可见字符
char *rtrim(char* str);
//去除字符串左右不可见字符
char *trim(char* str);