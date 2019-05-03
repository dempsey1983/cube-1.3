#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <termios.h>

#include "data_type.h"
#include "errno.h"
#include "alloc.h"
#include "string.h"
#include "basefunc.h"
#include "struct_deal.h"
#include "crypto_func.h"
#include "memdb.h"
#include "message.h"
#include "ex_module.h"

#include "sys_func.h"


//static struct timeval time_val={0,50*1000};

static int  state=0;

int get_password(const char *prompt,char * passwd);   

int proc_term_io_start(void * sub_proc,void * para);
int proc_keyset_info(void * sub_proc,void * para);

int term_io_init(void * sub_proc,void * para)
{
	system("stty erase ^H");
	return 0;
}

int term_io_start(void * sub_proc,void * para)
{
	int ret;
	int retval;
	void * recv_msg;
	void * send_msg;
	void * context;
	void * sock;
	BYTE uuid[DIGEST_SIZE];
	int i;
	int type;
	int subtype;

	print_cubeaudit("begin term_io %s!\n",ex_module_getname(sub_proc));
	
	while(1)
	{
		usleep(time_val.tv_usec);
		ret=ex_module_recvmsg(sub_proc,&recv_msg);
		if(ret>0)
		{
			if(recv_msg!=NULL)
			{
 				type=message_get_type(recv_msg);
				subtype=message_get_subtype(recv_msg);
			}
		}

		ret=proc_term_io_start(sub_proc,para);
		if(ret<0)
			return ret;
		continue;

/*
		if((type==DTYPE_TAC_KEY_MANAGE)&&(subtype==SUBTYPE_SESSION_KEYBLOB))
		{
			// the last step of key exchange
			proc_session_keyblob(sub_proc,recv_msg);
			continue;
		}
		if((type==DTYPE_TAC_KEY_MANAGE)&&(subtype==SUBTYPE_QUOTE_REPORT))
		{
			// the beginning of integrity check
			ex_module_sendmsg(sub_proc,recv_msg);
			continue;
		}
*/
	}

	return 0;
};


int proc_term_io_start(void * sub_proc,void * para)
{
	int ret;
	int i;
	int len;

	BYTE  buf[DIGEST_SIZE*16];
	void * send_msg;
	RECORD(MESSAGE,BASE_MSG) inputs;

	print_cubeaudit("begin proc term_io start\n");

//	ret=proc_share_data_getvalue("proc_name",proc_name);

	fgets(buf,DIGEST_SIZE*8,stdin);
	for(i=0;i<Strlen(buf);i++)
	{
		if(buf[i]=='\n')
		{
			buf[i]=0;
			break;
		}
	}

	inputs.message=buf;	

	send_msg=message_create(DTYPE_MESSAGE,SUBTYPE_BASE_MSG,NULL);
	if(send_msg==NULL)
		return -EINVAL;
	message_add_record(send_msg,&inputs);
	ex_module_sendmsg(sub_proc,send_msg);
	
	return 0;
}

char getch()
 {    
    char c;
    system("stty -echo");
    system("stty erase ^H");
    system("stty -icanon");
    c=getchar();
    system("stty icanon");
    system("stty echo");
    return c;
 }    

int get_password(const char *prompt,char * passwd)   
{
    static char buffer[DIGEST_SIZE];
    int i = 0;
    char letter = '\0';

    printf(prompt);
    while((i<DIGEST_SIZE-1)&&(letter!='\n'))
    //如果没有按回车并且达到最大长度 
    {
        letter = getch();
        if(letter == '\b')
        //如果是退格符，表示要删除前面输入的一个字符 
        {
            if(i>0)
            //如果以前输入自符 
            {
                passwd[--i] = '\0'; //从缓冲区中删除最有一个字符 
                putchar('\b'); //光标位置前移一个字符位置 
                putchar(' '); //将要删除的字符(回显的*)从屏幕中置为空白 
                putchar('\b'); //光标位置前移一个字符位置
            } 
            else
            {
                putchar(7); //响铃
            }
        }
        else if(letter != '\n')
        //如果按下回车 
        {
            passwd[i++] = letter;
            putchar('*');
        }
    }
    passwd[i] = '\0'; //设置字符串结束标志 
    return i;
}
