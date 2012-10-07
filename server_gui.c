#include<gtk/gtk.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<string.h>
#include<stdio.h>
#include<sys/select.h>
#include<arpa/inet.h>

#define PORT 2080

GtkWidget *window;
GtkWidget *frame;
GtkWidget *disconnect;
GtkWidget *sends;
GtkWidget *vbox;
GtkWidget *hbox;
GtkWidget *cli_count;
GtkWidget *clients;
GtkWidget *progress;
GtkWidget *fileframe;
GtkWidget *filelabel;
GtkWidget *browse;
GtkWidget *filebox;
GtkWidget *clientframe;
int sock,count=0;
struct clientstr
{
	int sockfd;
	struct sockaddr_in cli;
	struct clientstr *next;
}*client,*temp;

int Accept (int s,struct sockaddr* addr, socklen_t* addrlen, int msecs)
{
	struct timeval val;
	static int i=0;
	fd_set set;
	val.tv_sec = (msecs / 1000);
	val.tv_usec = (msecs % 1000) * 1000;
	FD_ZERO(&set);
	FD_SET(s, &set);
	printf(" %d\n",i++);
	switch (select(s + 1, &set, 0, 0, &val))
	{
		case 0: // timeout - treat it like an error.
			return -1;
		case 1: // success - input activity detected.
			return accept(s, addr, addrlen);
		default: // error
			return -1;
	}

}
int incoming(gpointer data)
{
	int clength = sizeof(struct sockaddr_in);
	char ip[40];
	struct clientstr *new=(struct clientstr *)malloc(sizeof(struct clientstr));
	new->sockfd=Accept(sock,(struct sockaddr *)&(new->cli),&clength,100);
	printf("%d ",new->sockfd);
	if(new->sockfd!=-1)
	{
		char string[40];
		new->next=NULL;
		if(client==NULL)
			client=new;
		else
		{
			temp=client;
			while(temp->next!=NULL)
				temp=temp->next;
			temp->next=new;
		}
		count++;
		sprintf(string,"%d clients are available",count);
		gtk_label_set_text(GTK_LABEL(cli_count),string);
		inet_ntop(AF_INET,(void *)&(new->cli.sin_addr),ip,clength);
		sprintf(string,"%d %s\n",new->sockfd,ip);
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(clients),string);
		
	}
	
	return 1;
}
void initialize()
{
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),"Server");
	gtk_widget_set_size_request(window,300,300);
	gtk_window_set_resizable(GTK_WINDOW(window),0);
	frame=gtk_frame_new(NULL);
	disconnect=gtk_button_new_with_label("Disconnect");
	sends=gtk_button_new_with_label("Send");
	cli_count=gtk_label_new("0");
	vbox=gtk_vbox_new(1,5);
	hbox=gtk_hbox_new(1,5);
	clients=gtk_combo_box_text_new();
	progress=gtk_progress_bar_new();
	fileframe=gtk_frame_new("Select file");
	filelabel=gtk_label_new(NULL);
	browse=gtk_button_new_with_label("Browse");
	filebox=gtk_hbox_new(0,0);
	clientframe=gtk_frame_new("Select a client");
}
void choosefile(GtkWidget *browse)
{
	GtkWidget *dialog;
	dialog=gtk_file_chooser_dialog_new("Open File",GTK_WINDOW(window),GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);
	if(gtk_dialog_run(GTK_DIALOG (dialog))==GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_label_set_text(GTK_LABEL(filelabel),filename);
		//strcpy(filename,file);
		gtk_widget_destroy (dialog);
		g_free(filename);
	}
}
int sendfile(GtkWidget *send)
{
	char *string=(char *)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(clients));

	if(string==NULL)
		return 0;
	int sockfd=0;
	int i=0;
	while(string[i]!=' ')
	{
		sockfd=sockfd*10+(int)(string[i]-48);
		i++;
	}


	if (gtk_label_get_text(GTK_LABEL(filelabel))!=NULL)
  	{
		char *filename=(char *)gtk_label_get_text(GTK_LABEL(filelabel));
		FILE *ptr;
		char c;
		int size=0;
		int loc=0;
		int d;
    		
		
		ptr=fopen(filename,"r");
		gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress),1);
		c='0';
		write(sockfd,&c,1);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress),"Sending File");
		while(fscanf(ptr,"%c",&c)!=EOF)
		{
			d=(int)c;
			write(sockfd,&d,sizeof(int));
		}
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress),"File transfer completed");
		d=-999;
		write(sockfd,&d,sizeof(int));
		fclose(ptr);
		//gtk_widget_destroy(status);
    		//g_free (filename);
		g_free(string);
		return 1;
  	}

	g_free(string);
	
	return 1;
}
void shuttoff(GtkWidget *window)
{
	char c='1';
	while(client!=NULL)
	{
		temp=client;
		client=client->next;
		write(temp->sockfd,&c,1);
		close(temp->sockfd);
		free(temp);
	}
	close(sock);
	gtk_main_quit();
}
void update()
{
	char ip[40];
	char string[40];
	int i;
	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(clients)); //for gtk3 and after
	//use below code for gtk before gtk3

	temp=client;
	while(temp!=NULL)
	{
		inet_ntop(AF_INET,(void *)&(temp->cli.sin_addr),ip,sizeof(struct sockaddr_in));
		sprintf(string,"%d %s\n",temp->sockfd,ip);
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(clients),string);
		temp=temp->next;
	}
	sprintf(string,"%d clients are available",count);
	gtk_label_set_text(GTK_LABEL(cli_count),string);
}
		

int remove_client(GtkWidget *disconnect)
{

	char *string;
	
	int fd=0;
	int i=0;
	char c='1';
	string=(char *)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(clients));
	if(string==NULL)
		return 0;
	//printf("x");
	while(string[i]!=' ')
	{
		printf("%c",string[i]);
		fd=fd*10+(int)(string[i]-48);
		i++;
	}

	write(fd,&c,1);
	//printf("Trying to close client");
	close(fd);
	//printf("test");
	if(fd==(client->sockfd))
	{
		//printf("if begins");
		temp=client;
		client=client->next;
		free(temp);
		//printf("ok");
	}
	else
	{
		//printf("else begins");
		struct clientstr *t;
		temp=client;
		while(temp->next->sockfd!=fd)
			temp=temp->next;
		t=temp->next;
		temp->next=temp->next->next;
		free(t);
	}
	//printf("done");
	count--;
	g_free(string);
	update();
	return 1;	
}
void signals()
{
	g_signal_connect(window,"destroy",G_CALLBACK(shuttoff),NULL);
	g_timeout_add_seconds(1,incoming,NULL);
	g_signal_connect(sends,"clicked",G_CALLBACK(sendfile),NULL);
	g_signal_connect(disconnect,"clicked",G_CALLBACK(remove_client),NULL);
	g_signal_connect(browse,"clicked",G_CALLBACK(choosefile),NULL);
}
void packing()
{
	gtk_container_add(GTK_CONTAINER(window),frame);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	gtk_box_pack_start(GTK_BOX(vbox),clientframe,0,0,5);
	gtk_container_add(GTK_CONTAINER(clientframe),clients);
	gtk_box_pack_start(GTK_BOX(vbox),fileframe,0,0,5);
	gtk_container_add(GTK_CONTAINER(fileframe),filebox);
	gtk_box_pack_start(GTK_BOX(filebox),filelabel,0,0,5);
	gtk_box_pack_start(GTK_BOX(filebox),browse,0,0,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,1,1,0);
	gtk_box_pack_start(GTK_BOX(hbox),sends,0,1,5);
	gtk_box_pack_start(GTK_BOX(hbox),disconnect,0,1,5);
	gtk_box_pack_start(GTK_BOX(vbox),cli_count,1,1,5);
	gtk_box_pack_start(GTK_BOX(vbox),progress,0,0,5);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress),0); 
	//gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress),"Disabled");
	gtk_widget_show_all(window);
}
int main(int argc,char *argv[])
{
	gtk_init(&argc,&argv);
	struct sockaddr_in serv;
	sock =  socket(AF_INET,SOCK_STREAM,0);
 	
	serv.sin_family = AF_INET;
	serv.sin_port = htons(PORT);
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sock,(struct sockaddr *)&serv, sizeof(serv));
	listen(sock,5);
	client=NULL;
	initialize();
	gtk_label_set_text(GTK_LABEL(cli_count),"Waiting for clients ...");
	signals();
	packing();
	gtk_main();
}
	
