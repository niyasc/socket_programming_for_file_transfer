#include<gtk/gtk.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>  
#include <arpa/inet.h>
#include <stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#define PORT 2080
GtkWidget *window;
GtkWidget *frame;
GtkWidget *Connect;
GtkWidget *vbox;
GtkWidget *getip;
GtkWidget *status;
GtkWidget *frameip;
GtkWidget *fstatus;

struct sockaddr_in serv;
int sock;
//initialize
void initialize()
{
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),"Client");
	gtk_widget_set_size_request(window,300,300);
	gtk_window_set_resizable(GTK_WINDOW(window),0);
	frame=gtk_frame_new(NULL);
	frameip=gtk_frame_new("Enter your IP here ");
	Connect=gtk_button_new_with_label("Connect");
	vbox=gtk_vbox_new(0,0);
	getip=gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(getip),"127.0.0.1");
	fstatus=gtk_label_new(NULL);
	status=gtk_label_new(NULL);
	
	sock = socket(AF_INET,SOCK_STREAM,0);
	serv.sin_port = htons(PORT);
	serv.sin_family = AF_INET;
}
char *getfilename()
{
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new("Select Location",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
	
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  	{
    		char *filename;
    		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    		printf("%s",(char *)filename);
    		//g_free (filename);
		//gtk_widget_destroy(prompt);
		gtk_widget_destroy (dialog);
		return filename;
  	}
			
}
int getconnect()
{
	FILE *ptr;
	gtk_label_set_text(GTK_LABEL(status),"Trying to connect...");
	char *ip=(char *)gtk_entry_get_text(GTK_ENTRY(getip));
	serv.sin_addr.s_addr = inet_addr(ip);
	if(connect(sock, (struct sockaddr *)&serv,sizeof(serv))==-1)
	{
		gtk_label_set_text(GTK_LABEL(status),"Failed to connect..");
		return 0;
	}
	else
		gtk_label_set_text(GTK_LABEL(status),"Server connection established");

}
char Read(int msecs)
{
	struct timeval val;
	//static int i=0;
	fd_set set;
	char c;
	int fd=sock;

	val.tv_sec = (msecs / 1000);
	val.tv_usec = (msecs % 1000) * 1000;

	FD_ZERO(&set);
	FD_SET(fd, &set);
	//printf(" %d\n",i++);
	switch (select(fd + 1, &set,0,0,&val))
	{
		case 0: // timeout - treat it like an error.
			return 0;
		case 1: // success - input activity detected.
			read(fd, &c, 1);
			return c;
		default: // error
			return 0;
	}

}
int check(gpointer data)
{
	char c;
	int d;
	FILE *ptr;
	char *fname;
	static int flag=0;
	//printf("hi");
	if(!flag)
	{
		c=Read(100);
		if(c==0)
			return 1;
	}
	else
		return 0;
	flag=1;
	switch(c)
	{
		case '0':fname=getfilename();
			printf("%s\n",fname);
			printf("Opening file\n");
			gtk_label_set_text(GTK_LABEL(fstatus),NULL);	
			ptr=fopen(fname,"w");
			while(1)
			{
					
				read(sock,&d,sizeof(int));
				//printf("%s",buf);
				if(d==(-999))
				{
					fclose(ptr);
					printf("\nFile closed\n");
					flag=0;
					free(fname);
					gtk_label_set_text(GTK_LABEL(fstatus),"File Saved");
					return 1;
				}
				//read(sock,&c,1);
				printf("%c",d);
				fprintf(ptr,"%c",d);
			}

			//return 1;
		case '1':close(sock);
			//gtk_widget_destroy_all(window);
			gtk_main_quit();
			exit(0);
	}
	return 1;
}

void signals()
{
	g_signal_connect(window,"destroy",gtk_main_quit,NULL);
	g_signal_connect(Connect,"clicked",G_CALLBACK(getconnect),NULL);
	g_timeout_add(100,check,NULL);
}
void packing()
{
	gtk_container_add(GTK_CONTAINER(window),frame);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	gtk_box_pack_start(GTK_BOX(vbox),frameip,0,0,5);
	gtk_box_pack_start(GTK_BOX(vbox),Connect,0,0,5);
	gtk_box_pack_start(GTK_BOX(vbox),status,0,0,5);
	gtk_box_pack_start(GTK_BOX(vbox),fstatus,0,0,10);
	gtk_container_add(GTK_CONTAINER(frameip),getip);
	
}

int main(int argc,char *argv[])
{
	gtk_init(&argc,&argv);
	initialize();
	signals();
	packing();
	gtk_widget_show_all(window);
	gtk_main();
}
