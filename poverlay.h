#ifndef POVERLAY_H
#define POVERLAY_H

#define BUFSIZE 512
void overlay_main_loop(VOID);
void instance_thread(LPVOID);
void get_answer_to_request(LPTSTR, LPTSTR, LPDWORD);


#endif // POVERLAY_H
