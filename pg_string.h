#ifndef PG_STRING_H
#define PG_STRING_H
char **ctokenize(const char *str, const char *delim);
int ctoken_counter(const char *str, const char *delim);
int skip_delim(const char *str, const char *delim);

char **stokenize(const char *str, const char *dmstr);
int char_counter(const char *str, char character);
int stoken_counter(const char *str, const char *dmstr);
int strskp(const char *str, const char *skstr);
char * sstrtok(char *str, const char *dmstr);
int strcnt(const char * str);
char * astrcat(char ** strarray, char * delim, int start, int end);
char **ctokenize_pair(char * str, char delim);
char * strepclean(char * dirty_str, char dirt);

#endif
