#ifndef _HELPER_H
#define _HELPER_H

void do_process(int cfd);
int get_line(int sockfd, char *buf, int size);

void http_request(const char* request, int cfd);
void send_respond_head(int cfd, int no, const char* desp, const char* type, long len);
void send_file(int cfd, const char* filename);
void send_dir(int cfd, const char* dirname);
void send_error(int cfd, int status, char *title, char *text);

void encode_str(char* to, int tosize, const char* from);
void decode_str(char *to, char *from);
const char *get_file_type(const char *name);

#endif
