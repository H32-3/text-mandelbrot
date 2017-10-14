#include <stdio.h>	//this is for the spritnf function (like printf but on a string (pointer))
#include <unistd.h>	//for sleep()
#include <caca.h>
#include <pthread.h>
#define NUM_THREADS	5

caca_canvas_t* canvas;
caca_display_t* display;
caca_dither_t* dither;
caca_event_t event;
int screen_width;
int screen_height;

#define bpp 24
#define depth 3

#define rmask 0xff0000
#define gmask 0x00ff00
#define bmask 0x0000ff
#define amask 0

pthread_t threads[NUM_THREADS];

typedef struct CNumber{
	float r;//real
	float i;//imaginary
}CNumber;
typedef struct send{
	int y;//imaginary
	int threadc;
	float zoom;
	float cameraX;
	float cameraY;
}send;

uint8_t fb[3*(WIDTH*HEIGHT)];

/*i could make this a function but then multiple threads would be calling the 
 * samefunction and it would slow down a litle bit */
#define pixel(x,y,val) \
	fb[((x)*3+0)+((y)*WIDTH)*3]=((val)&0x0000ff)>>0 , \
	fb[((x)*3+1)+((y)*WIDTH)*3]=((val)&0x00ff00)>>8 , \
	fb[((x)*3+2)+((y)*WIDTH)*3]=((val)&0xff0000)>>16; \

#define brighten(x,y,factor) \
	fb[(x*3+0)+(y*WIDTH)*3]*=factor , \
	fb[(x*3+1)+(y*WIDTH)*3]*=factor , \
	fb[(x*3+2)+(y*WIDTH)*3]*=factor ; \

int maxfun=10000; 

CNumber add(CNumber a,CNumber b){
	CNumber c;
	c.r=a.r+b.r;
	c.i=a.i+b.i;
	return(c);
}

CNumber multiply(CNumber a,CNumber b){
	CNumber c;
	c.r=a.r*b.r-a.i*b.i;
	c.i=a.r*b.i+a.i*b.r;
	return(c);
}
void init_caca(){
	canvas=caca_create_canvas(0,0);
	display=caca_create_display(canvas);
	screen_width=caca_get_canvas_width(canvas);
	screen_height=caca_get_canvas_height(canvas);
	dither=caca_create_dither(	bpp,
					WIDTH,
					HEIGHT,
					depth * WIDTH,
					rmask,
					gmask,
					bmask,
					amask	);	return;
}
float absf(float in){
	if(in < 0)	in*=(-1.0);
	return(in);
}
#define LINESPERTHREAD 24
void lineF(send *input){
	int max;
	CNumber mathin,mathout,mathplace;
	for(int lines=0;lines<LINESPERTHREAD;lines++)
		if(lines+input->y<WIDTH)
			for(int loop=0;loop<WIDTH;loop++){
				mathplace.r=loop*((float)(input->zoom)/WIDTH)-input->zoom/2-input->cameraX,
					mathplace.i=(input->y+lines)*((float)(input->zoom)/WIDTH)-input->zoom/2-input->cameraY;
				mathin.i=0,mathin.r=0,max=0;
				pixel(loop,input->y+lines,0);
				while(max++<maxfun){
					mathout=add(multiply(mathin,mathin),mathplace); 
					if( (absf(mathout.r)<.00000001) && (absf(mathout.i)<.00000001) ){
						pixel(loop,input->y+lines,0);
						break;
					}else if((absf(mathout.r)>100000.0)||(absf(mathout.i)>100000.0)){ 
						pixel(loop,input->y+lines,60-max);
						break;
					}else{ 
						if( (absf(mathout.r)==absf(mathin.r)) && (absf(mathout.i)==absf(mathin.i)) ){
							pixel(loop,input->y+lines,0);
							break;
						}
						mathin=mathout;
					}
				}
//				brighten(loop,input->y+lines,1);
			}
}

void wfb(){
	caca_get_event(display,CACA_EVENT_RESIZE|CACA_EVENT_QUIT,&event,1);
		if(caca_get_event_type(&event)==CACA_EVENT_RESIZE)
			screen_width=caca_get_event_resize_width(&event),
			screen_height=caca_get_event_resize_height(&event);
	caca_dither_bitmap(canvas,0,0,screen_width,screen_height,dither,&fb);
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

}

int main(){
	int potition=0;
	init_caca();
	float zoom=6,cameraX,cameraY;
#ifdef MANUAL
#else
	cameraX=0.71339;
	cameraY=-0.47385;
#endif
	send messages[NUM_THREADS];
	while(1){
		for(int y=0;y<HEIGHT-4;y+=NUM_THREADS*LINESPERTHREAD){
			for (int thread=0;thread<NUM_THREADS;thread++){
				messages[thread].zoom=zoom,messages[thread].cameraX=cameraX,messages[thread].cameraY=cameraY;
				messages[thread].y=y+thread*LINESPERTHREAD;
				pthread_create(&threads[thread], NULL, &lineF,&messages[thread]);
			}
			for(int temp=0;temp<NUM_THREADS;temp++)
				pthread_join(threads[temp],NULL );
		}
#ifdef MANUAL
#else
		zoom/=1.005;
#endif
		wfb();
#ifndef MANUAL
		if(zoom<0.000002)break;
#endif
		potition++;
	}
}
