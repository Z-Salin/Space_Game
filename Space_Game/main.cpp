#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const int W=800, H=600;

// Player
float pX=400,pY=80,pAngle=90; // angle in degrees, 90=facing up
float pSpeed=4.0f, rotSpeed=5.0f;
int score=0,lives=3,level=1;
float damage=0;
bool gameOver=false, autoFire=false;
int highScore=5000, fireCD=0;

// Player bullets (directional)
#define MAX_B 20
struct Bullet{float x,y,dx,dy;bool on;}bul[MAX_B];
float bSpd=7.0f;

// Enemy bullets
#define MAX_EB 20
struct EBul{float x,y,dx,dy;bool on;}ebul[MAX_EB];
float ebSpd=3.5f;
int eFireCD=0;

// Enemies (take 3 hits)
#define MAX_E 4
struct Enemy{float x,y,spd;int hp;bool on;}ene[MAX_E];

// Meteors (take 3 hits)
#define MAX_M 6
struct Meteor{float x,y,r,spd,angle;int hp;bool on;}met[MAX_M];

// Explosions
#define MAX_EX 12
struct Explo{float x,y,t;bool on;}fx[MAX_EX];

// Stars
#define MAX_S 80
struct Star{float x,y;}stars[MAX_S];

// Blackhole - has gravity pull!
float bhX=650,bhY=450,bhAng=0;
float bhRadius=50, bhGravity=0.15f;

// Key states for smooth movement
bool keyW=false,keyA=false,keyS=false,keyD=false;
bool kUp=false,kDown=false,kLeft=false,kRight=false;

int recovTimer=0;

void spawnEnemy(int i);
void spawnMeteor(int i);
void addFX(float x,float y);
void playerFire();

// Midpoint Circle Algorithm
void midCircle(float cx,float cy,int r){
    int x=0,y=r,d=1-r;
    glBegin(GL_POINTS);
    while(x<=y){
        glVertex2f(cx+x,cy+y);glVertex2f(cx-x,cy+y);
        glVertex2f(cx+x,cy-y);glVertex2f(cx-x,cy-y);
        glVertex2f(cx+y,cy+x);glVertex2f(cx-y,cy+x);
        glVertex2f(cx+y,cy-x);glVertex2f(cx-y,cy-x);
        if(d<0)d+=2*x+3; else{d+=2*(x-y)+5;y--;}
        x++;
    }
    glEnd();
}

void drawText(float x,float y,const char*s,void*f){
    glRasterPos2f(x,y);
    for(int i=0;s[i];i++)glutBitmapCharacter(f,s[i]);
}

void init(){
    glClearColor(1,1,1,1);
    glMatrixMode(GL_PROJECTION);glLoadIdentity();
    gluOrtho2D(0,W,0,H);glMatrixMode(GL_MODELVIEW);
    srand((unsigned)time(NULL));
    for(int i=0;i<MAX_B;i++)bul[i].on=false;
    for(int i=0;i<MAX_EB;i++)ebul[i].on=false;
    for(int i=0;i<MAX_E;i++){ene[i].on=true;spawnEnemy(i);}
    for(int i=0;i<MAX_M;i++){met[i].on=true;spawnMeteor(i);}
    for(int i=0;i<MAX_EX;i++)fx[i].on=false;
    for(int i=0;i<MAX_S;i++){stars[i].x=rand()%W;stars[i].y=rand()%H;}
}

void spawnEnemy(int i){
    ene[i].x=50+rand()%(W-100);
    ene[i].y=H+30+rand()%300;
    ene[i].spd=1.0f+(level*0.2f)+(rand()%15)/10.0f;
    ene[i].hp=2+level/3; // gets tougher with level
    ene[i].on=true;
}

void spawnMeteor(int i){
    met[i].x=rand()%W; met[i].y=H+50+rand()%400;
    met[i].r=10+rand()%20; met[i].spd=0.5f+(rand()%20)/10.0f;
    met[i].angle=(float)(rand()%360); met[i].hp=3; met[i].on=true;
}

void addFX(float x,float y){
    for(int i=0;i<MAX_EX;i++)
        if(!fx[i].on){fx[i].x=x;fx[i].y=y;fx[i].t=25;fx[i].on=true;return;}
}

void playerFire(){
    for(int i=0;i<MAX_B;i++){
        if(!bul[i].on){
            float rad=pAngle*(float)M_PI/180.0f;
            bul[i].dx=cos(rad)*bSpd;
            bul[i].dy=sin(rad)*bSpd;
            bul[i].x=pX+cos(rad)*40;
            bul[i].y=pY+sin(rad)*40;
            bul[i].on=true;
            return;
        }
    }
}

void enemyFire(float ex,float ey){
    for(int i=0;i<MAX_EB;i++){
        if(!ebul[i].on){
            ebul[i].x=ex;ebul[i].y=ey-25;
            float dx=pX-ex,dy=pY-ey;
            float len=sqrt(dx*dx+dy*dy);if(len<1)len=1;
            ebul[i].dx=(dx/len)*ebSpd;
            ebul[i].dy=(dy/len)*ebSpd;
            ebul[i].on=true;return;
        }
    }
}

// Apply blackhole gravity to a point, returns true if sucked in
bool bhGrav(float&x,float&y,float&vx,float&vy,float pull){
    float dx=bhX-x,dy=bhY-y;
    float dist=sqrt(dx*dx+dy*dy);
    if(dist<bhRadius*0.3f)return true; // sucked in
    if(dist<bhRadius*3){
        float force=pull/(dist*0.1f);
        vx+=dx/dist*force; vy+=dy/dist*force;
    }
    return false;
}

// Draw functions
void drawPlayer(){
    glColor3f(0,0,0);
    glPushMatrix();
    glTranslatef(pX,pY,0);
    glRotatef(pAngle-90,0,0,1); // -90 because 90deg=up in our system
    glBegin(GL_LINE_LOOP);
    glVertex2f(-10,-20);glVertex2f(10,-20);glVertex2f(10,20);glVertex2f(-10,20);
    glEnd();
    glBegin(GL_LINE_LOOP);glVertex2f(-10,20);glVertex2f(10,20);glVertex2f(0,40);glEnd();
    glBegin(GL_LINE_LOOP);glVertex2f(-10,-10);glVertex2f(-10,15);glVertex2f(-30,-10);glEnd();
    glBegin(GL_LINE_LOOP);glVertex2f(10,-10);glVertex2f(10,15);glVertex2f(30,-10);glEnd();
    glBegin(GL_LINE_LOOP);glVertex2f(-5,-30);glVertex2f(5,-30);glVertex2f(5,-20);glVertex2f(-5,-20);glEnd();
    glBegin(GL_LINE_LOOP);glVertex2f(-5,5);glVertex2f(5,5);glVertex2f(0,15);glEnd();
    glPopMatrix();
}

void drawEnemyShip(float ex,float ey){
    glColor3f(0,0,0);
    glPushMatrix();glTranslatef(ex,ey,0);
    glBegin(GL_LINE_LOOP);glVertex2f(0,-25);glVertex2f(-12,10);glVertex2f(12,10);glEnd();
    glBegin(GL_LINE_LOOP);glVertex2f(-12,10);glVertex2f(12,10);glVertex2f(12,18);glVertex2f(-12,18);glEnd();
    glBegin(GL_LINE_LOOP);glVertex2f(-12,5);glVertex2f(-12,15);glVertex2f(-28,15);glEnd();
    glBegin(GL_LINE_LOOP);glVertex2f(12,5);glVertex2f(12,15);glVertex2f(28,15);glEnd();
    glBegin(GL_LINE_LOOP);glVertex2f(-5,-5);glVertex2f(5,-5);glVertex2f(0,-15);glEnd();
    glPopMatrix();
}

void drawMeteor(float mx,float my,float r,float a){
    glColor3f(0,0,0);
    glPushMatrix();glTranslatef(mx,my,0);glRotatef(a,0,0,1);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-r*0.6f,-r);glVertex2f(r*0.4f,-r*0.8f);
    glVertex2f(r,-r*0.3f);glVertex2f(r*0.8f,r*0.5f);
    glVertex2f(r*0.3f,r);glVertex2f(-r*0.5f,r*0.7f);
    glVertex2f(-r,r*0.2f);glVertex2f(-r*0.7f,-r*0.4f);
    glEnd();glPopMatrix();
}

void drawFX(float ex,float ey,float t){
    glColor3f(0,0,0);float s=(25-t)*1.2f;
    glPushMatrix();glTranslatef(ex,ey,0);
    for(int i=0;i<8;i++){
        float a=i*45.0f*(float)M_PI/180.0f;
        glBegin(GL_LINES);glVertex2f(0,0);glVertex2f(cos(a)*s,sin(a)*s);glEnd();
    }
    if(s>10)midCircle(0,0,(int)(s*0.6f));
    glPopMatrix();
}

void drawBlackhole(){
    glColor3f(0,0,0);
    glPushMatrix();glTranslatef(bhX,bhY,0);glRotatef(bhAng,0,0,1);
    for(int r=10;r<=50;r+=8)midCircle(0,0,r);
    for(int i=0;i<6;i++){
        float a=(i*60+bhAng)*(float)M_PI/180;
        glBegin(GL_LINES);glVertex2f(0,0);glVertex2f(cos(a)*55,sin(a)*55);glEnd();
    }
    // Draw gravity range indicator (dotted circle)
    midCircle(0,0,(int)(bhRadius*3));
    glPopMatrix();
}

void drawStars(){
    glColor3f(0.75f,0.75f,0.75f);glPointSize(2);
    glBegin(GL_POINTS);
    for(int i=0;i<MAX_S;i++)glVertex2f(stars[i].x,stars[i].y);
    glEnd();glPointSize(1);
}

void drawBullets(){
    glColor3f(0,0,0);glLineWidth(2);
    for(int i=0;i<MAX_B;i++){
        if(!bul[i].on)continue;
        float len=12,mag=sqrt(bul[i].dx*bul[i].dx+bul[i].dy*bul[i].dy);
        float nx=0,ny=len;
        if(mag>0.01f){nx=(bul[i].dx/mag)*len;ny=(bul[i].dy/mag)*len;}
        glBegin(GL_LINES);glVertex2f(bul[i].x,bul[i].y);glVertex2f(bul[i].x+nx,bul[i].y+ny);glEnd();
    }
    glLineWidth(1);
}

void drawEBullets(){
    glColor3f(0.3f,0.3f,0.3f);glLineWidth(2);
    for(int i=0;i<MAX_EB;i++){
        if(!ebul[i].on)continue;
        float len=10,mag=sqrt(ebul[i].dx*ebul[i].dx+ebul[i].dy*ebul[i].dy);
        float nx=0,ny=-len;
        if(mag>0.01f){nx=(ebul[i].dx/mag)*len;ny=(ebul[i].dy/mag)*len;}
        glBegin(GL_LINES);glVertex2f(ebul[i].x,ebul[i].y);glVertex2f(ebul[i].x+nx,ebul[i].y+ny);glEnd();
    }
    glLineWidth(1);
}

void drawHUD(){
    glColor3f(0,0,0);char buf[64];
    sprintf(buf,"SCORE: %05d",score);
    drawText(20,H-30,buf,GLUT_BITMAP_HELVETICA_18);
    drawText(20,H-55,"LIVES:",GLUT_BITMAP_HELVETICA_18);
    for(int i=0;i<lives;i++){
        float hx=110+i*25,hy=H-52;
        glBegin(GL_LINE_LOOP);
        glVertex2f(hx,hy+5);glVertex2f(hx-5,hy+10);glVertex2f(hx-8,hy+7);glVertex2f(hx-5,hy+3);
        glVertex2f(hx,hy-2);glVertex2f(hx+5,hy+3);glVertex2f(hx+8,hy+7);glVertex2f(hx+5,hy+10);
        glEnd();
    }
    sprintf(buf,"LEVEL: %02d",level);drawText(W-150,H-30,buf,GLUT_BITMAP_HELVETICA_18);
    sprintf(buf,"HIGH: %05d",highScore);drawText(W-150,H-55,buf,GLUT_BITMAP_HELVETICA_18);

    // Auto-fire indicator
    if(autoFire) drawText(W/2-30,H-30,"[AUTO FIRE]",GLUT_BITMAP_HELVETICA_12);

    // Damage bar
    float bW=200,bH=15,bX=W/2-bW/2,bY=15;
    glBegin(GL_LINE_LOOP);
    glVertex2f(bX,bY);glVertex2f(bX+bW,bY);glVertex2f(bX+bW,bY+bH);glVertex2f(bX,bY+bH);
    glEnd();
    float fW=(damage/100)*bW;
    glBegin(GL_QUADS);
    glVertex2f(bX,bY);glVertex2f(bX+fW,bY);glVertex2f(bX+fW,bY+bH);glVertex2f(bX,bY+bH);
    glEnd();
    drawText(bX-70,bY+2,"DAMAGE:",GLUT_BITMAP_HELVETICA_12);

    // Controls help
    drawText(10,10,"W/S:Move A/D:Rotate Arrows:Strafe SPACE:Fire F:AutoFire",GLUT_BITMAP_HELVETICA_12);
}

void drawGameOver(){
    glColor3f(0,0,0);
    drawText(W/2-80,H/2+20,"GAME OVER",GLUT_BITMAP_TIMES_ROMAN_24);
    char buf[64];sprintf(buf,"Final Score: %d",score);
    drawText(W/2-70,H/2-10,buf,GLUT_BITMAP_HELVETICA_18);
    drawText(W/2-90,H/2-40,"Press R to Restart",GLUT_BITMAP_HELVETICA_18);
}

bool boxHit(float x1,float y1,float s1,float x2,float y2,float s2){
    return(x1-s1<x2+s2&&x1+s1>x2-s2&&y1-s1<y2+s2&&y1+s1>y2-s2);
}

void update(int v){
    if(gameOver){glutPostRedisplay();glutTimerFunc(32,update,0);return;}

    // Smooth movement from held keys
    float rad=pAngle*(float)M_PI/180;
    if(keyW){pX+=cos(rad)*pSpeed;pY+=sin(rad)*pSpeed;} // forward
    if(keyS){pX-=cos(rad)*pSpeed*0.6f;pY-=sin(rad)*pSpeed*0.6f;} // backward (slower)
    if(keyA)pAngle+=rotSpeed; // rotate left
    if(keyD)pAngle-=rotSpeed; // rotate right
    if(kUp){pY+=pSpeed;} if(kDown){pY-=pSpeed;}
    if(kLeft){pX-=pSpeed;} if(kRight){pX+=pSpeed;}

    // Clamp player position
    if(pX<30)pX=30; if(pX>W-30)pX=W-30;
    if(pY<40)pY=40; if(pY>H-60)pY=H-60;

    // Normalize angle
    if(pAngle>=360)pAngle-=360; if(pAngle<0)pAngle+=360;

    // Auto fire
    if(autoFire){fireCD++;if(fireCD>8){playerFire();fireCD=0;}}

    // Stars
    for(int i=0;i<MAX_S;i++){stars[i].y-=0.3f;if(stars[i].y<0){stars[i].y=H;stars[i].x=rand()%W;}}

    // Blackhole rotation
    bhAng+=0.8f;if(bhAng>=360)bhAng-=360;

    // Move player bullets + blackhole gravity on them
    for(int i=0;i<MAX_B;i++){
        if(!bul[i].on)continue;
        bhGrav(bul[i].x,bul[i].y,bul[i].dx,bul[i].dy,bhGravity*0.5f);
        bul[i].x+=bul[i].dx;bul[i].y+=bul[i].dy;
        if(bul[i].y>H||bul[i].y<0||bul[i].x<0||bul[i].x>W)bul[i].on=false;
    }

    // Move enemy bullets
    for(int i=0;i<MAX_EB;i++){
        if(!ebul[i].on)continue;
        ebul[i].x+=ebul[i].dx;ebul[i].y+=ebul[i].dy;
        if(ebul[i].y>H||ebul[i].y<0||ebul[i].x<0||ebul[i].x>W)ebul[i].on=false;
    }

    // Move enemies + blackhole gravity
    for(int i=0;i<MAX_E;i++){
        if(!ene[i].on)continue;
        float vx=0,vy=-ene[i].spd;
        if(bhGrav(ene[i].x,ene[i].y,vx,vy,bhGravity)){
            addFX(ene[i].x,ene[i].y);spawnEnemy(i);score+=75;continue;
        }
        ene[i].x+=vx;ene[i].y+=vy;
        if(ene[i].y<-40)spawnEnemy(i);
    }

    // Enemy fire
    eFireCD++;
    if(eFireCD>70){
        int c[MAX_E],n=0;
        for(int i=0;i<MAX_E;i++)if(ene[i].on&&ene[i].y>50&&ene[i].y<H-50)c[n++]=i;
        if(n>0)enemyFire(ene[c[rand()%n]].x,ene[c[rand()%n]].y);
        eFireCD=0;
    }

    // Move meteors
    for(int i=0;i<MAX_M;i++){
        if(!met[i].on)continue;
        met[i].y-=met[i].spd;met[i].angle+=0.5f;
        if(met[i].y<-50)spawnMeteor(i);
    }

    // Explosions
    for(int i=0;i<MAX_EX;i++)
        if(fx[i].on){fx[i].t-=1;if(fx[i].t<=0)fx[i].on=false;}

    // Damage recovery
    recovTimer++;
    if(recovTimer>90){if(damage>0)damage-=2;if(damage<0)damage=0;recovTimer=0;}

    // Blackhole gravity on player!
    float pvx=0,pvy=0;
    if(bhGrav(pX,pY,pvx,pvy,bhGravity*0.8f)){
        addFX(pX,pY);lives=0;gameOver=true;
    } else { pX+=pvx;pY+=pvy; }

    // Player bullet vs Enemy
    for(int i=0;i<MAX_B;i++){
        if(!bul[i].on)continue;
        for(int j=0;j<MAX_E;j++){
            if(!ene[j].on)continue;
            if(boxHit(bul[i].x,bul[i].y,5,ene[j].x,ene[j].y,20)){
                bul[i].on=false;
                ene[j].hp--;
                if(ene[j].hp<=0){
                    PlaySound(TEXT("E:\\Space Game\\SpaceGame\\sound.wav"), NULL, SND_ASYNC | SND_FILENAME);
                    addFX(ene[j].x,ene[j].y);score+=100;
                    if(score>highScore)highScore=score;
                    level=1+score/500;spawnEnemy(j);
                }
                break;
            }
        }
    }

    // Player bullet vs Meteor
    for(int i=0;i<MAX_B;i++){
        if(!bul[i].on)continue;
        for(int j=0;j<MAX_M;j++){
            if(!met[j].on)continue;
            if(boxHit(bul[i].x,bul[i].y,5,met[j].x,met[j].y,met[j].r*0.7f)){
                bul[i].on=false;met[j].hp--;
                if(met[j].hp<=0){addFX(met[j].x,met[j].y);score+=50;
                    if(score>highScore)highScore=score;level=1+score/500;spawnMeteor(j);}
                break;
            }
        }
    }

    // Enemy bullet vs Player
    for(int i=0;i<MAX_EB;i++){
        if(!ebul[i].on)continue;
        if(boxHit(ebul[i].x,ebul[i].y,4,pX,pY,18)){
            ebul[i].on=false;damage+=20;
            if(damage>=100){damage=0;lives--;addFX(pX,pY);if(lives<=0)gameOver=true;}
        }
    }

    // Player vs Enemy
    for(int j=0;j<MAX_E;j++){
        if(!ene[j].on)continue;
        if(boxHit(pX,pY,20,ene[j].x,ene[j].y,18)){
            addFX(ene[j].x,ene[j].y);spawnEnemy(j);damage+=30;
            if(damage>=100){damage=0;lives--;if(lives<=0)gameOver=true;}
        }
    }

    // Player vs Meteor (instant death)
    for(int j=0;j<MAX_M;j++){
        if(!met[j].on)continue;
        if(boxHit(pX,pY,18,met[j].x,met[j].y,met[j].r*0.7f)){
            addFX(pX,pY);lives=0;gameOver=true;}
    }

    // Enemy vs Meteor
    for(int i=0;i<MAX_E;i++){
        if(!ene[i].on)continue;
        for(int j=0;j<MAX_M;j++){
            if(!met[j].on)continue;
            if(boxHit(ene[i].x,ene[i].y,18,met[j].x,met[j].y,met[j].r*0.7f)){
                addFX(ene[i].x,ene[i].y);spawnEnemy(i);score+=50;}
        }
    }

    glutPostRedisplay();glutTimerFunc(32,update,0);
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);glLoadIdentity();
    drawStars();drawBlackhole();
    for(int i=0;i<MAX_M;i++)if(met[i].on)drawMeteor(met[i].x,met[i].y,met[i].r,met[i].angle);
    for(int i=0;i<MAX_E;i++)if(ene[i].on)drawEnemyShip(ene[i].x,ene[i].y);
    drawBullets();drawEBullets();
    for(int i=0;i<MAX_EX;i++)if(fx[i].on)drawFX(fx[i].x,fx[i].y,fx[i].t);
    if(!gameOver)drawPlayer();
    drawHUD();if(gameOver)drawGameOver();
    glutSwapBuffers();
}

// Key DOWN handlers
void onKeyDown(unsigned char key,int x,int y){
    if(key=='w'||key=='W')keyW=true;
    if(key=='s'||key=='S')keyS=true;
    if(key=='a'||key=='A')keyA=true;
    if(key=='d'||key=='D')keyD=true;
    if(key==' '&&!gameOver)playerFire();
    if(key=='f'||key=='F')autoFire=!autoFire;
    if((key=='r'||key=='R')&&gameOver){
        pX=400;pY=80;pAngle=90;score=0;lives=3;level=1;damage=0;
        gameOver=false;autoFire=false;
        for(int i=0;i<MAX_B;i++)bul[i].on=false;
        for(int i=0;i<MAX_EB;i++)ebul[i].on=false;
        for(int i=0;i<MAX_E;i++)spawnEnemy(i);
        for(int i=0;i<MAX_M;i++)spawnMeteor(i);
        for(int i=0;i<MAX_EX;i++)fx[i].on=false;
    }
    if(key==27)exit(0);
}

void onKeyUp(unsigned char key,int x,int y){
    if(key=='w'||key=='W')keyW=false;
    if(key=='s'||key=='S')keyS=false;
    if(key=='a'||key=='A')keyA=false;
    if(key=='d'||key=='D')keyD=false;
}

void onSpecDown(int key,int x,int y){
    if(key==GLUT_KEY_UP)kUp=true;
    if(key==GLUT_KEY_DOWN)kDown=true;
    if(key==GLUT_KEY_LEFT)kLeft=true;
    if(key==GLUT_KEY_RIGHT)kRight=true;
}

void onSpecUp(int key,int x,int y){
    if(key==GLUT_KEY_UP)kUp=false;
    if(key==GLUT_KEY_DOWN)kDown=false;
    if(key==GLUT_KEY_LEFT)kLeft=false;
    if(key==GLUT_KEY_RIGHT)kRight=false;
}

int main(int argc,char**argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(W,H);glutInitWindowPosition(100,100);
    glutCreateWindow("Space Galaxy Fighter - B&W");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(onKeyDown);glutKeyboardUpFunc(onKeyUp);
    glutSpecialFunc(onSpecDown);glutSpecialUpFunc(onSpecUp);
    glutTimerFunc(32,update,0);
    glutMainLoop();return 0;
}
