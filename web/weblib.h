#define LOG_PREFX "[WebSubSystem]:"
char *doit(char *text,const char *item_str);
char * http_get(const char *url,int timeout) ;
char * http_post(const char *url,const char *post_str,int timeout);

