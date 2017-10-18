#include <stdio.h> //puts()
#include <string.h> //strcmp()
#include <time.h>
#include <caca.h>
#include <pthread.h>

#define NUM_THREADS	20

caca_canvas_t* canvas;
caca_display_t* display;
caca_dither_t* dither;
caca_event_t event;

int screen_width;
int screen_height;

float zoom=6,cameraX,cameraY;
float fps=0;

#define bpp 24
#define depth 3

#define rmask 0xff0000
#define gmask 0x00ff00
#define bmask 0x0000ff
#define amask 0

pthread_t threads[NUM_THREADS];

typedef struct CNumber{ //complex numbers
	float r;//real
	float i;//imaginary
}CNumber;

typedef struct send{ //the data to send to each thread
	int y;//imaginary
	float* zoom;
	float* cameraX;
	float* cameraY;
}send;

int autozoom=3;

uint8_t fb[3*(WIDTH*HEIGHT)]; //the frame buffer

#define pixel(x,y,val) /*draw a pixel on the frame buffer*/\
	fb[((x)*3+0)+((y)*WIDTH)*3]=((val)&0x0000ff)>>0 , \
	fb[((x)*3+1)+((y)*WIDTH)*3]=((val)&0x00ff00)>>8 , \
	fb[((x)*3+2)+((y)*WIDTH)*3]=((val)&0xff0000)>>16; \

int maxfun=100; 

CNumber add(CNumber a,CNumber b){ //add two complex numbers
	CNumber c;
	c.r=a.r+b.r;
	c.i=a.i+b.i;
	return(c);
}

CNumber multiply(CNumber a,CNumber b){//multiply two complex numbers
	CNumber c;
	c.r=a.r*b.r-a.i*b.i;
	c.i=a.r*b.i+a.i*b.r;
	return(c);
}

#define abs(in) (((in)<0)?in*-1.0:in)

void init_caca(){//init libcaca
	canvas=caca_create_canvas(0,0);
	display=caca_create_display(canvas);
	screen_width=caca_get_canvas_width(canvas);
	screen_height=caca_get_canvas_height(canvas);
}

#define LINESPERTHREAD 50
void *lineF(void *inputv){
	send* input=inputv;
	int max;
	CNumber mathin,mathout,mathplace;
	for(int lines=0;lines<LINESPERTHREAD;lines++)
		if(lines+input->y<HEIGHT)
			for(int loop=0;loop<WIDTH;loop++){
				mathplace.r=loop*((float)(*input->zoom)/WIDTH)-(*input->zoom)/2-(*input->cameraX),
					mathplace.i=(input->y+lines)*((float)(*input->zoom)/WIDTH)-(*input->zoom)/2-(*input->cameraY);
				mathin.i=0,mathin.r=0,max=0;
				pixel(loop,input->y+lines,0);
				while(max++<maxfun){
					mathout=add(multiply(mathin,mathin),mathplace); 
					if( (abs(mathout.r)<.00000001) && (abs(mathout.i)<.00000001) ){
						pixel(loop,input->y+lines,0);
						break;
					}else if((abs(mathout.r)>100000.0)||(abs(mathout.i)>100000.0)){ 
						pixel(loop,input->y+lines,60-max);
						break;
					}else{ 
						if( (abs(mathout.r)==abs(mathin.r)) && (abs(mathout.i)==abs(mathin.i)) ){
							pixel(loop,input->y+lines,0);
							break;
						}
						mathin=mathout;
					}
				}
			}
	return 0;
}

unsigned char wfb(){//also handle events like CACA_EVENT_QUIT
	caca_get_event(display,CACA_EVENT_KEY_PRESS|CACA_EVENT_RESIZE|CACA_EVENT_QUIT,&event,1);
	if(caca_get_event_type(&event)==CACA_EVENT_RESIZE){
		screen_width=caca_get_event_resize_width(&event);
		screen_height=caca_get_event_resize_height(&event);
		return 2;
	}else if(caca_get_event_type(&event)==CACA_EVENT_QUIT){
		return 1;
	}else if(caca_get_event_key_ch(&event)=='q'){
		return 1;
	}else if(!autozoom){
		if(caca_get_event_key_ch(&event)=='+'){
			zoom/=1.05;
			return 2;
		}else if(caca_get_event_key_ch(&event)=='-'){
			zoom*=1.05;
			return 2;
		}else if(caca_get_event_key_ch(&event)=='h'){
			cameraX+=.05*zoom;
			return 2;
		}else if(caca_get_event_key_ch(&event)=='l'){
			cameraX-=.05*zoom;
			return 2;
		}else if(caca_get_event_key_ch(&event)=='k'){
			cameraY+=.05*zoom;
			return 2;
		}else if(caca_get_event_key_ch(&event)=='j'){
			cameraY-=.05*zoom;
			return 2;
		}
	}
	caca_dither_bitmap(canvas,0,0,screen_width,screen_height,dither,&fb);
	if(autozoom)
		caca_printf(canvas,0,0,"FPS:%02.01f",fps);
	else 
		caca_put_str(canvas,0,0,"FPS:--.-");
	caca_refresh_display(display);
	caca_free_dither(dither);
	dither=caca_create_dither(	bpp,
			WIDTH,
			HEIGHT,
			depth * WIDTH,
			rmask,
			gmask,
			bmask,
			amask	);
	return 0;
}//return value:  0) no event 1)quit the program 2)redraw if manual

int main(int argc,char* argd[]){
	if(argc!=2)
		return puts("you need to specify only one option \n --manual , -m	for manual control\n --autozoom , -a for automatic zoom\n");
	if(strcmp(argd[1],"-a")==0||strcmp(argd[1],"--autozoom")==0)
		autozoom=1;
	else if(strcmp(argd[1],"-m")==0||strcmp(argd[1],"--manual")==0)
		autozoom=0;
	else
		return puts("you need to specify only one option \n --manual , -m	for manual control\n --autozoom , -a for automatic zoom\n");	
	init_caca();
	cameraX=0.71339;
	cameraY=-0.47385;
	send messages[NUM_THREADS];
	for (int thread=0;thread<NUM_THREADS;thread++)
		messages[thread].zoom=&zoom,messages[thread].cameraX=&cameraX,messages[thread].cameraY=&cameraY;
	clock_t clockS;
	while(1){
		if(autozoom)
			clockS=clock();
		for(int y=0;y<HEIGHT;y+=NUM_THREADS*LINESPERTHREAD){
			for (int thread=0;thread<NUM_THREADS;thread++){
				messages[thread].y=y+thread*LINESPERTHREAD;
				pthread_create(&threads[thread], NULL, &lineF,&messages[thread]);
			}
			for (int thread=0;thread<NUM_THREADS;thread++)
				pthread_join(threads[thread],NULL );
		}
		if(autozoom){
			zoom/=1.009;
			if(wfb()==1)
				break;
		}else{
			int whatdo=1;
			while(whatdo==1)
				switch(wfb()){
					case 0:
						whatdo=1;//dont render new image
						usleep(30000);
						break;
					case 1:
						whatdo=2;//exit
						break;
					case 2:
						whatdo=3;//we got a key, render a new image
						break;
				}
			if(whatdo==2)
				break;
		}

		if(zoom<0.000002&&autozoom)break;
		if(autozoom)
			fps=1.0/((float)(clock()-clockS)/CLOCKS_PER_SEC);
	}
	caca_free_dither(dither);
	caca_free_canvas(canvas);
	caca_free_display(display);
}
