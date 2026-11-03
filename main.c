#define WIN32_LEAN_AND_MEAN
#ifndef TEST_MODE
#include <windows.h>
#include <windowsx.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#define MAX_PASSWORD 4096
#define PUZZLE_RULES 25
#define RULES_TOTAL 26
#define ID_STUDENT 1001
#define ID_SIGNIN 1002
#define ID_PASS 1003
#define ID_RESET 1004
#define ID_CONFIRM 1005
#define ID_SAVE 1006
#define TIMER_GAME 2101

static char student_id[32]="";
static char captcha[8]="q7z";
static int math_answer=7, binary_answer=101;
static const char *names[PUZZLE_RULES]={
 "At least 5 characters",
 "Contains a digit (0-9)",
 "Contains an uppercase letter",
 "Contains one of ! @ # $",
 "Contains sponsor: engineer / computer / cpe",
 "ALL decimal digits sum to 101",
 "Contains an English month",
 "Contains a Roman numeral letter (I V X L C D M)",
 "Roman letter values ADD to 40",
 "Contains the CAPTCHA shown on screen",
 "At least 2 elements; atomic numbers ADD to 67",
 "Contains a leap year (YYYY)",
 "Contains the math answer (7)",
 "Contains a color: red / blue / green / yellow",
 "Total password length is prime",
 "Contains three consecutive letters (abc, bcd...)",
 "Contains a valid 24-hour time (HH:MM)",
 "Number of # = number of sponsor matches",
 "Contains a palindrome: radar / level / civic",
 "Uppercase letter count = lowercase letter count",
 "No identical letters adjacent (case-insensitive)",
 "Total password length MOD 5 = 2",
 "Contains binary challenge answer 101",
 "Contains each vowel: a e i o u",
 "Contains YOUR signed-in student ID"
};

static int contains_ci(const char*s,const char*t){size_t n=strlen(t);for(;*s;s++){size_t i=0;while(i<n&&s[i]&&tolower((unsigned char)s[i])==tolower((unsigned char)t[i]))i++;if(i==n)return 1;}return 0;}
static int count_ci(const char*s,const char*t){int cnt=0;size_t n=strlen(t);for(;*s;s++){size_t i=0;while(i<n&&s[i]&&tolower((unsigned char)s[i])==tolower((unsigned char)t[i]))i++;if(i==n)cnt++;}return cnt;}
static int sum_digits(const char*s){int n=0;for(;*s;s++)if(*s>='0'&&*s<='9')n+=*s-'0';return n;}
static int roman_letter(char c){switch(c){case 'I':return 1;case 'V':return 5;case 'X':return 10;case 'L':return 50;case 'C':return 100;case 'D':return 500;case 'M':return 1000;default:return 0;}}
static int roman_sum(const char*s){int n=0;for(;*s;s++)n+=roman_letter(*s);return n;}
static int prime(int n){if(n<2)return 0;for(int i=2;i<=n/i;i++)if(n%i==0)return 0;return 1;}
static int leap(int y){return y%4==0&&(y%100!=0||y%400==0);}
static int contains_leap(const char*s){for(int i=0;s[i];i++)if((i==0||!isdigit((unsigned char)s[i-1]))&&isdigit((unsigned char)s[i])&&isdigit((unsigned char)s[i+1])&&isdigit((unsigned char)s[i+2])&&isdigit((unsigned char)s[i+3])&&!isdigit((unsigned char)s[i+4])){int y=(s[i]-'0')*1000+(s[i+1]-'0')*100+(s[i+2]-'0')*10+s[i+3]-'0';if(y>=1000&&leap(y))return 1;}return 0;}
static int time_ok(const char*s){for(int i=0;s[i];i++)if((i==0||!isdigit((unsigned char)s[i-1]))&&isdigit((unsigned char)s[i])&&isdigit((unsigned char)s[i+1])&&s[i+2]==':'&&isdigit((unsigned char)s[i+3])&&isdigit((unsigned char)s[i+4])&&!isdigit((unsigned char)s[i+5])){int h=10*(s[i]-'0')+s[i+1]-'0',m=10*(s[i+3]-'0')+s[i+4]-'0';if(h<24&&m<60)return 1;}return 0;}
static int sequence3(const char*s){for(int i=0;s[i]&&s[i+1]&&s[i+2];i++){int a=tolower((unsigned char)s[i]),b=tolower((unsigned char)s[i+1]),c=tolower((unsigned char)s[i+2]);if(a>='a'&&c<='z'&&b==a+1&&c==b+1)return 1;}return 0;}
static int no_adjacent(const char*s){for(int i=0;s[i]&&s[i+1];i++)if(isalpha((unsigned char)s[i])&&isalpha((unsigned char)s[i+1])&&tolower((unsigned char)s[i])==tolower((unsigned char)s[i+1]))return 0;return 1;}
static int valid_student(const char*s){if(strlen(s)!=11||strncmp(s,"690705010",9))return 0;return isdigit((unsigned char)s[9])&&isdigit((unsigned char)s[10]);}
/* Use case-sensitive element symbols: a capital followed by its lowercase letter.
   One-character elements are recognized when the uppercase letter is NOT part of a
   two-letter symbol. Every character is STILL independently counted by Roman rules. */
static const struct {const char*symbol;int num;} element_table[]={
 {"H",1},{"He",2},{"Li",3},{"Be",4},{"B",5},{"C",6},{"N",7},{"O",8},{"F",9},{"Ne",10},
 {"Na",11},{"Mg",12},{"Al",13},{"Si",14},{"P",15},{"S",16},{"Cl",17},{"Ar",18},{"K",19},{"Ca",20},
 {"Sc",21},{"Ti",22},{"V",23},{"Cr",24},{"Mn",25},{"Fe",26},{"Co",27},{"Ni",28},{"Cu",29},{"Zn",30},
 {"Ga",31},{"Ge",32},{"As",33},{"Se",34},{"Br",35},{"Kr",36},{"Rb",37},{"Sr",38},{"Y",39},{"Zr",40},
 {"Nb",41},{"Mo",42},{"Tc",43},{"Ru",44},{"Rh",45},{"Pd",46},{"Ag",47},{"Cd",48},{"In",49},{"Sn",50},
 {"Sb",51},{"Te",52},{"I",53},{"Xe",54},{"Cs",55},{"Ba",56},{"La",57},{"Ce",58},{"Pr",59},{"Nd",60},
 {"Pm",61},{"Sm",62},{"Eu",63},{"Gd",64},{"Tb",65},{"Dy",66},{"Ho",67},{"Er",68},{"Tm",69},{"Yb",70},
 {"Lu",71},{"Hf",72},{"Ta",73},{"W",74},{"Re",75},{"Os",76},{"Ir",77},{"Pt",78},{"Au",79},{"Hg",80},
 {"Tl",81},{"Pb",82},{"Bi",83},{"Po",84},{"At",85},{"Rn",86},{"Fr",87},{"Ra",88},{"Ac",89},{"Th",90},
 {"Pa",91},{"U",92},{"Np",93},{"Pu",94},{"Am",95},{"Cm",96},{"Bk",97},{"Cf",98},{"Es",99},{"Fm",100},
 {"Md",101},{"No",102},{"Lr",103},{"Rf",104},{"Db",105},{"Sg",106},{"Bh",107},{"Hs",108},{"Mt",109},{"Ds",110},
 {"Rg",111},{"Cn",112},{"Nh",113},{"Fl",114},{"Mc",115},{"Lv",116},{"Ts",117},{"Og",118}
};
static void element_stats(const char*s,int*sum,int*count){
 *sum=*count=0;
 for(int i=0;s[i];){
  if(!isupper((unsigned char)s[i])){i++;continue;}
  int best=0,length=0;
  for(size_t j=0;j<sizeof(element_table)/sizeof(element_table[0]);j++){
   int n=(int)strlen(element_table[j].symbol);
   if(n>length&&strncmp(s+i,element_table[j].symbol,n)==0){best=element_table[j].num;length=n;}
  }
  if(best){*sum+=best;(*count)++;i+=length;}else i++;
 }
}
static int rule_ok(int r,const char*s){
 int upper=0,lower=0,atom=0,atoms=0;size_t len=strlen(s);
 static const char*months[]={"january","february","march","april","may","june","july","august","september","october","november","december"};
 switch(r){
 case 0:return len>=5;
 case 1:for(;*s;s++)if(isdigit((unsigned char)*s))return 1;return 0;
 case 2:for(;*s;s++)if(isupper((unsigned char)*s))return 1;return 0;
 case 3:return strpbrk(s,"!@#$")!=NULL;
 case 4:return contains_ci(s,"engineer")||contains_ci(s,"computer")||contains_ci(s,"cpe");
 case 5:return sum_digits(s)==101;
 case 6:for(int i=0;i<12;i++)if(contains_ci(s,months[i]))return 1;return 0;
 case 7:for(;*s;s++)if(roman_letter(*s))return 1;return 0;
 case 8:return roman_sum(s)==40;
 case 9:return strstr(s,captcha)!=NULL;
 case 10:element_stats(s,&atom,&atoms);return atoms>=2&&atom==67;
 case 11:return contains_leap(s);
 case 12:{char t[16];snprintf(t,sizeof(t),"%d",math_answer);return strstr(s,t)!=NULL;}
 case 13:return contains_ci(s,"red")||contains_ci(s,"blue")||contains_ci(s,"green")||contains_ci(s,"yellow");
 case 14:return prime((int)len);
 case 15:return sequence3(s);
 case 16:return time_ok(s);
 case 17:{int h=0;for(const char*p=s;*p;p++)h+=*p=='#';return h==count_ci(s,"engineer")+count_ci(s,"computer")+count_ci(s,"cpe");}
 case 18:return contains_ci(s,"radar")||contains_ci(s,"level")||contains_ci(s,"civic");
 case 19:for(;*s;s++){upper+=isupper((unsigned char)*s)!=0;lower+=islower((unsigned char)*s)!=0;}return upper==lower;
 case 20:return no_adjacent(s);
 case 21:return len%5==2;
 case 22:{char t[32];snprintf(t,sizeof(t),"%d",binary_answer);return strstr(s,t)!=NULL;}
 case 23:for(const char*v="aeiou";*v;v++){char t[2]={*v,0};if(!contains_ci(s,t))return 0;}return 1;
 case 24:return valid_student(student_id)&&strstr(s,student_id)!=NULL;
 default:return 0;
 }
}
static int save_student(const char*id){
 if(!valid_student(id))return 0;
#ifdef _WIN32
 _mkdir("data");
#else
 mkdir("data",0700);
#endif
 FILE*f=fopen("data/students.txt","a+");if(!f)return 0;
 char line[64];rewind(f);
 while(fgets(line,sizeof(line),f)){line[strcspn(line,"\r\n")]=0;if(!strcmp(line,id)){fclose(f);return 1;}}
 if(fseek(f,0,SEEK_END)!=0){fclose(f);return 0;}
 int ok=fprintf(f,"%s\n",id)>0;if(fclose(f)!=0)ok=0;return ok;
}

#ifdef TEST_MODE
int main(void){
 strcpy(student_id,"69070501025");
 int at=0,n=0;element_stats("Xe",&at,&n);if(at!=54||n!=1||roman_sum("XeVohII")!=17)return 1;
 element_stats("FeNb",&at,&n);if(at!=67||n!=2)return 2;
 if(!valid_student(student_id)||valid_student("690705010AA"))return 3;
 printf("Logic smoke tests passed. Rule count: %d\n",PUZZLE_RULES);
 return 0;
}
#else
#include <windows.h>
#include <windowsx.h>
#define ID_POPUP 1501
static HWND hwndMain,hStudent,hSign,hPass,hReset,hSave,hConfirm,hInfo,hPopup;
static HFONT fontBody,fontTitle,fontSmall,fontBig;
static HBRUSH editBrush;
static int signed_in=0,unlocked=1,final_stage=0,won=0,scroll_pos=0;
static int leftX=46,leftW=520,rightX=610,rightW=520,cardTop=25,cardBottom=640;
static int rulesTop=172,ruleHeight=56,ruleVisible=6,originalY=214,originalH=125;
static char password[MAX_PASSWORD]="",original[MAX_PASSWORD]="";
static unsigned long elapsed_seconds=0;
static DWORD last_tick=0;
static int restore_in_progress=0;
/* Each student has an independent saved game. Password content is saved locally. */
typedef struct {char signature[8];unsigned version;char id[12];unsigned level;unsigned final_stage;unsigned won;unsigned long seconds;char password[MAX_PASSWORD];char original[MAX_PASSWORD];} SaveData;
static void game_path(char *out,size_t cap){snprintf(out,cap,"data/save_%s.bin",student_id);}
static int save_game(void){
 if(!signed_in||!valid_student(student_id))return 0;
#ifdef _WIN32
 _mkdir("data");
#endif
 SaveData d={0};memcpy(d.signature,"PWGAME6",7);d.version=1;strncpy(d.id,student_id,11);
 d.level=(unsigned)unlocked;d.final_stage=(unsigned)final_stage;d.won=(unsigned)won;d.seconds=elapsed_seconds;
 memcpy(d.password,password,MAX_PASSWORD);memcpy(d.original,original,MAX_PASSWORD);
 char path[96],temp[104];game_path(path,sizeof(path));snprintf(temp,sizeof(temp),"%s.tmp",path);
 FILE *f=fopen(temp,"wb");if(!f)return 0;
 int ok=(fwrite(&d,sizeof(d),1,f)==1);if(fclose(f)!=0)ok=0;
 if(!ok){remove(temp);return 0;}
 /* remove then rename is compatible with older Windows C runtimes */
 remove(path);if(rename(temp,path)!=0)return 0;return 1;
}
static int load_game(void){
 char path[96];game_path(path,sizeof(path));FILE*f=fopen(path,"rb");if(!f)return 0;
 SaveData d;int ok=fread(&d,sizeof(d),1,f)==1;fclose(f);
 if(!ok||memcmp(d.signature,"PWGAME6",7)||d.version!=1||strncmp(d.id,student_id,12))return 0;
 if(d.level<1||d.level>PUZZLE_RULES||d.final_stage>1||d.won>1)return 0;
 if(!memchr(d.password,0,MAX_PASSWORD)||!memchr(d.original,0,MAX_PASSWORD))return 0;
 unlocked=(int)d.level;final_stage=(int)d.final_stage;won=(int)d.won;
 elapsed_seconds=d.seconds;memcpy(password,d.password,MAX_PASSWORD);memcpy(original,d.original,MAX_PASSWORD);
 return 1;
}
static void tick_time(void){if(!signed_in||won)return;DWORD now=GetTickCount();
 DWORD diff=now-last_tick;if(diff>=1000){unsigned long sec=diff/1000;elapsed_seconds+=sec;last_tick+=sec*1000;}}

static COLORREF bg=RGB(8,15,28),white=RGB(18,30,49),green=RGB(65,214,158),red=RGB(251,112,105),blue=RGB(39,185,239);
static int clamp(int x,int lo,int hi){return x<lo?lo:(x>hi?hi:x);}
static void sf(HWND h,HFONT f){SendMessageA(h,WM_SETFONT,(WPARAM)f,TRUE);}
static void txt(HWND h,const char*s){SetWindowTextA(h,s);}
static void rounded(HDC dc,int x,int y,int w,int h,int rad,COLORREF color){HBRUSH b=CreateSolidBrush(color);HPEN pen=CreatePen(PS_SOLID,1,color);HGDIOBJ ob=SelectObject(dc,b),op=SelectObject(dc,pen);RoundRect(dc,x,y,x+w,y+h,rad,rad);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(b);DeleteObject(pen);}
static void outline(HDC dc,int x,int y,int w,int h,COLORREF c){HPEN p=CreatePen(PS_SOLID,1,c);HGDIOBJ o=SelectObject(dc,p);HGDIOBJ b=SelectObject(dc,GetStockObject(NULL_BRUSH));RoundRect(dc,x,y,x+w,y+h,14,14);SelectObject(dc,b);SelectObject(dc,o);DeleteObject(p);}
static void label(HDC dc,const char*s,int x,int y,int w,int h,COLORREF color,HFONT f,UINT align){RECT r={x,y,x+w,y+h};SetBkMode(dc,TRANSPARENT);SetTextColor(dc,color);SelectObject(dc,f);DrawTextA(dc,s,-1,&r,DT_WORDBREAK|DT_NOPREFIX|align);}
static LRESULT CALLBACK pass_proc(HWND h,UINT m,WPARAM w,LPARAM l){if(final_stage&&(m==WM_PASTE||m==WM_CONTEXTMENU||m==WM_DROPFILES))return 0;return CallWindowProcA((WNDPROC)GetWindowLongPtrA(h,GWLP_USERDATA),h,m,w,l);}
static LRESULT CALLBACK popproc(HWND w,UINT m,WPARAM wp,LPARAM lp){
 if(m==WM_ERASEBKGND)return 1;
 if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);RECT r;GetClientRect(w,&r);HBRUSH dark=CreateSolidBrush(bg);FillRect(dc,&r,dark);DeleteObject(dark);
 rounded(dc,8,6,r.right-16,r.bottom-12,20,RGB(18,30,49));
 label(dc,"CAPTCHA",28,79,r.right-56,45,RGB(70,210,246),fontBig,DT_CENTER);
 rounded(dc,51,145,r.right-102,65,16,RGB(11,22,39));
 char shown[64];int k=0;for(int i=0;captcha[i]&&k<55;i++){shown[k++]=captcha[i];if(captcha[i+1])shown[k++]=' ';}shown[k]=0;
 label(dc,shown,52,160,r.right-104,42,RGB(245,182,94),fontBig,DT_CENTER);
 EndPaint(w,&ps);return 0;
 }
 return DefWindowProcA(w,m,wp,lp);
}
static void position_popup(void){if(!hPopup||!hwndMain)return;
 RECT r;GetWindowRect(hwndMain,&r);int x=r.right-370,y=r.top+95;
 SetWindowPos(hPopup,HWND_TOP,x,y,325,350,SWP_NOACTIVATE);
}
static void popup_sync(void){if(hPopup)ShowWindow(hPopup,SW_HIDE);}
static void layout(void);

static void update_game(void){
 if(!signed_in||restore_in_progress)return;
 if(!final_stage)GetWindowTextA(hPass,password,MAX_PASSWORD);
 while(!final_stage&&unlocked<PUZZLE_RULES){int all=1;for(int i=0;i<unlocked;i++)if(!rule_ok(i,password)){all=0;break;}if(!all)break;unlocked++;}
 int all=1,passed=0;for(int i=0;i<unlocked;i++){int ok=rule_ok(i,password);passed+=ok;if(!ok)all=0;}
 if(unlocked==PUZZLE_RULES&&all&&!final_stage){
  final_stage=1;strcpy(original,password);txt(hPass,"");
  EnableWindow(hPass,TRUE);ShowWindow(hConfirm,SW_SHOW);layout();
  save_game();
  MessageBoxA(hwndMain,"All 25 rules passed! Your original password remains visible, but cannot be selected or copied. Retype it below (paste is blocked).","Rule 26",MB_OK|MB_ICONINFORMATION);
 }
 char status[128];snprintf(status,sizeof(status),"Level %d   |   %d/%d passed",unlocked,passed,unlocked);txt(hInfo,status);
 save_game();popup_sync();InvalidateRect(hwndMain,NULL,FALSE);
}
static void layout(void){
 if(!hwndMain||!hStudent)return;
 RECT cr;GetClientRect(hwndMain,&cr);int W=cr.right,H=cr.bottom;
 int margin=clamp(W/40,18,52), gap=clamp(W/35,28,60);
 cardTop=clamp(H/30,14,34);cardBottom=H-cardTop;
 int innerW=W-2*margin-80;
 leftW=clamp((innerW-gap)*46/100,280,900);
 leftX=margin+28;rightX=leftX+leftW+gap+24;
 rightW=W-margin-28-rightX;
 if(rightW<240){rightW=240;rightX=W-margin-28-rightW;leftW=rightX-leftX-gap-24;}
 int inputW=leftW-22;
 MoveWindow(hStudent,leftX,222,inputW,38,TRUE);
 originalY=420;originalH=clamp(H/7,88,145);
 if(final_stage){
  MoveWindow(hPass,leftX,originalY+originalH+65,inputW,clamp(H/9,70,120),TRUE);
  MoveWindow(hConfirm,leftX,originalY+originalH+65+clamp(H/9,70,120)+12,inputW,44,TRUE);
 }else{
  MoveWindow(hPass,leftX,395,inputW,clamp(H/3,120,240),TRUE);
 }
 MoveWindow(hSign,leftX,417,inputW,48,TRUE);
 MoveWindow(hReset,leftX,cardBottom-83,(inputW-12)/2,42,TRUE);
 MoveWindow(hSave,leftX+(inputW-12)/2+12,cardBottom-83,(inputW-12)/2,42,TRUE);
 MoveWindow(hInfo,rightX,rulesTop-52,rightW,23,TRUE);
 ruleHeight=clamp(H/11,58,76);rulesTop=267;
 ruleVisible=clamp((cardBottom-rulesTop-105)/ruleHeight,1,PUZZLE_RULES);
 scroll_pos=clamp(scroll_pos,0,unlocked>ruleVisible?unlocked-ruleVisible:0);
 position_popup();
}
static LRESULT CALLBACK wndproc(HWND w,UINT m,WPARAM wp,LPARAM lp){switch(m){
 case WM_CREATE:{
  hwndMain=w;HINSTANCE ins=(HINSTANCE)GetWindowLongPtrA(w,GWLP_HINSTANCE);
  fontBody=CreateFontA(-17,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Segoe UI");
  fontTitle=CreateFontA(-21,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Segoe UI");
  fontSmall=CreateFontA(-13,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Segoe UI");
  fontBig=CreateFontA(-34,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Segoe UI");
  editBrush=CreateSolidBrush(RGB(12,24,41));
  hStudent=CreateWindowExA(0,"EDIT","",WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL,0,0,0,0,w,(HMENU)ID_STUDENT,ins,NULL);SendMessageA(hStudent,EM_SETLIMITTEXT,11,0);sf(hStudent,fontBody);
  hSign=CreateWindowA("BUTTON","ENTER THE PROTOCOL",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW,0,0,0,0,w,(HMENU)ID_SIGNIN,ins,NULL);sf(hSign,fontBody);
  hPass=CreateWindowExA(0,"EDIT","",WS_CHILD|WS_TABSTOP|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL,0,0,0,0,w,(HMENU)ID_PASS,ins,NULL);sf(hPass,fontBody);SendMessageA(hPass,EM_SETLIMITTEXT,MAX_PASSWORD-1,0);
  SetWindowLongPtrA(hPass,GWLP_USERDATA,(LONG_PTR)SetWindowLongPtrA(hPass,GWLP_WNDPROC,(LONG_PTR)pass_proc));
  hConfirm=CreateWindowA("BUTTON","VERIFY FINAL KEY",WS_CHILD|WS_TABSTOP|BS_OWNERDRAW,0,0,0,0,w,(HMENU)ID_CONFIRM,ins,NULL);
  hReset=CreateWindowA("BUTTON","SWITCH USER",WS_CHILD|WS_TABSTOP|BS_OWNERDRAW,0,0,0,0,w,(HMENU)ID_RESET,ins,NULL);
  hSave=CreateWindowA("BUTTON","SAVE PROGRESS",WS_CHILD|WS_TABSTOP|BS_OWNERDRAW,0,0,0,0,w,(HMENU)ID_SAVE,ins,NULL);
  hInfo=CreateWindowA("STATIC","",WS_CHILD,0,0,0,0,w,NULL,ins,NULL);sf(hInfo,fontSmall);
  WNDCLASSA cls={0};cls.lpfnWndProc=popproc;cls.hInstance=ins;cls.lpszClassName="CaptchaSideCard";cls.hCursor=LoadCursor(NULL,IDC_ARROW);RegisterClassA(&cls);
  hPopup=CreateWindowExA(WS_EX_TOOLWINDOW,"CaptchaSideCard","CAPTCHA",WS_POPUP,0,0,325,290,w,NULL,ins,NULL);
  layout();return 0;
 }
 case WM_TIMER:if(wp==TIMER_GAME&&signed_in){tick_time();if(elapsed_seconds%10==0)save_game();InvalidateRect(w,NULL,FALSE);}return 0;
 case WM_CLOSE:tick_time();save_game();DestroyWindow(w);return 0;
 case WM_MOVE:position_popup();return 0;
 case WM_SIZE:layout();InvalidateRect(w,NULL,FALSE);return 0;
 case WM_SHOWWINDOW:if(!wp&&hPopup)ShowWindow(hPopup,SW_HIDE);else if(wp&&hPopup)popup_sync();break;
 case WM_MOUSEWHEEL:if(signed_in){scroll_pos=clamp(scroll_pos-GET_WHEEL_DELTA_WPARAM(wp)/WHEEL_DELTA*2,0,unlocked>ruleVisible?unlocked-ruleVisible:0);InvalidateRect(w,NULL,FALSE);}return 0;
 case WM_COMMAND:
  if(LOWORD(wp)==ID_SIGNIN&&HIWORD(wp)==BN_CLICKED){
   char id[32];GetWindowTextA(hStudent,id,sizeof(id));
   if(!valid_student(id)){MessageBoxA(w,"Student ID must match 690705010XX.","Invalid student ID",MB_OK|MB_ICONWARNING);return 0;}
   if(!save_student(id)){MessageBoxA(w,"Cannot save data/students.txt.","File error",MB_OK|MB_ICONERROR);return 0;}
   strcpy(student_id,id);signed_in=1;unlocked=1;final_stage=won=scroll_pos=0;elapsed_seconds=0;
   password[0]=original[0]=0;
   int restored=load_game();
   EnableWindow(hStudent,FALSE);EnableWindow(hSign,FALSE);
   ShowWindow(hPass,SW_SHOW);ShowWindow(hSign,SW_HIDE);
   ShowWindow(hReset,SW_SHOW);ShowWindow(hSave,SW_SHOW);ShowWindow(hInfo,SW_SHOW);
   ShowWindow(hConfirm,final_stage?SW_SHOW:SW_HIDE);
   restore_in_progress=1;txt(hPass,final_stage?"":password);restore_in_progress=0;
   last_tick=GetTickCount();SetTimer(w,TIMER_GAME,1000,NULL);
   layout();update_game();SetFocus(hPass);
   if(restored)MessageBoxA(w,"Saved game loaded. Your progress and play time have been restored.","Welcome back",MB_OK|MB_ICONINFORMATION);
   return 0;
  }
  if(LOWORD(wp)==ID_PASS&&HIWORD(wp)==EN_CHANGE){if(signed_in&&!final_stage)update_game();return 0;}
  if(LOWORD(wp)==ID_CONFIRM&&HIWORD(wp)==BN_CLICKED){char entered[MAX_PASSWORD];GetWindowTextA(hPass,entered,MAX_PASSWORD);if(!strcmp(original,entered)){tick_time();won=1;save_game();MessageBoxA(w,"YOU WIN! All 26 rules complete.","VICTORY",MB_OK|MB_ICONINFORMATION);}else{MessageBoxA(w,"Password incorrect. Try again.","Try again",MB_OK|MB_ICONWARNING);txt(hPass,"");}InvalidateRect(w,NULL,FALSE);return 0;}
  if(LOWORD(wp)==ID_SAVE&&HIWORD(wp)==BN_CLICKED){tick_time();MessageBoxA(w,save_game()?"Game saved successfully.":"Unable to save game.","Save Game",MB_OK);return 0;}
  if(LOWORD(wp)==ID_RESET&&HIWORD(wp)==BN_CLICKED){tick_time();save_game();KillTimer(w,TIMER_GAME);signed_in=0;unlocked=1;final_stage=won=scroll_pos=0;password[0]=original[0]=student_id[0]=0;txt(hStudent,"");txt(hPass,"");EnableWindow(hStudent,TRUE);EnableWindow(hSign,TRUE);ShowWindow(hPass,SW_HIDE);ShowWindow(hSign,SW_SHOW);ShowWindow(hConfirm,SW_HIDE);ShowWindow(hReset,SW_HIDE);ShowWindow(hSave,SW_HIDE);ShowWindow(hInfo,SW_HIDE);layout();popup_sync();InvalidateRect(w,NULL,TRUE);return 0;}
  break;
 case WM_DRAWITEM:{DRAWITEMSTRUCT*item=(DRAWITEMSTRUCT*)lp;
  if(item->CtlID==ID_SIGNIN||item->CtlID==ID_RESET||item->CtlID==ID_SAVE||item->CtlID==ID_CONFIRM){
   RECT rc=item->rcItem;int width=rc.right-rc.left,height=rc.bottom-rc.top;
   COLORREF fill=(item->CtlID==ID_RESET)?RGB(33,47,67):(item->CtlID==ID_SAVE?RGB(26,69,87):RGB(26,158,205));
   if(item->itemState&ODS_SELECTED)fill=RGB(19,101,137);
   rounded(item->hDC,0,0,width,height,14,fill);
   char b[64];GetWindowTextA(item->hwndItem,b,sizeof(b));
   label(item->hDC,b,3,10,width-6,height-14,RGB(244,251,255),fontBody,DT_CENTER);
   return TRUE;}break;}
 case WM_CTLCOLOREDIT:{HDC dc=(HDC)wp;SetBkColor(dc,RGB(12,24,41));SetTextColor(dc,RGB(220,242,255));return (LRESULT)editBrush;}
 case WM_CTLCOLORSTATIC:{HDC dc=(HDC)wp;SetBkColor(dc,white);SetTextColor(dc,RGB(155,182,208));return (LRESULT)GetStockObject(BLACK_BRUSH);}
 case WM_ERASEBKGND:return 1;
 case WM_PAINT:{
  PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);RECT bounds;GetClientRect(w,&bounds);
  int W=bounds.right,H=bounds.bottom;
  HDC mem=CreateCompatibleDC(dc);HBITMAP bmp=CreateCompatibleBitmap(dc,W,H);HGDIOBJ oldBmp=SelectObject(mem,bmp);
  HBRUSH back=CreateSolidBrush(bg);FillRect(mem,&bounds,back);DeleteObject(back);
  /* Cyber terminal masthead */
  rounded(mem,20,14,W-40,70,20,RGB(15,27,46));
  rounded(mem,38,30,40,40,12,RGB(19,61,83));
  label(mem,">_",43,38,34,25,RGB(78,219,250),fontTitle,DT_CENTER);
  label(mem,"PASSWORD_PROTOCOL",91,28,W-340,29,RGB(238,246,255),fontTitle,DT_LEFT);
  label(mem,"C EDITION  /  SYSTEM V7",93,57,W-360,18,RGB(104,145,170),fontSmall,DT_LEFT);
  rounded(mem,W-190,35,149,26,15,RGB(20,70,62));
  label(mem,"SYSTEM ONLINE",W-184,40,137,18,RGB(103,227,175),fontSmall,DT_CENTER);
  int top=106,left=leftX-16,right=rightX-16;
  int lw=leftW+24,rw=rightW+32;
  rounded(mem,left,top,lw,cardBottom-top+3,20,RGB(17,30,49));
  rounded(mem,right,top,rw,cardBottom-top+3,20,RGB(17,30,49));
  outline(mem,left,top,lw,cardBottom-top+3,RGB(36,62,85));
  outline(mem,right,top,rw,cardBottom-top+3,RGB(36,62,85));
  label(mem,signed_in?"PASSWORD EDITOR":"IDENTITY VERIFICATION",leftX,130,leftW,32,RGB(236,248,255),fontTitle,DT_LEFT);
  label(mem,signed_in?"Build a key that satisfies every rule":"Sign in to start the challenge",leftX,160,leftW,22,RGB(130,164,191),fontSmall,DT_LEFT);
  label(mem,"STUDENT ID",leftX,190,leftW,24,RGB(113,172,200),fontSmall,DT_LEFT);
  rounded(mem,leftX-3,215,leftW-16,50,12,RGB(11,23,40));
  outline(mem,leftX-3,215,leftW-16,50,RGB(43,83,107));
  if(signed_in){
   char info[120],timebuf[80];unsigned long sec=elapsed_seconds;
   snprintf(info,sizeof(info),"LEVEL %02d / 25",unlocked);
   label(mem,info,leftX,295,leftW/2,31,RGB(80,220,252),fontTitle,DT_LEFT);
   snprintf(timebuf,sizeof(timebuf),"TIME  %02lu:%02lu:%02lu",sec/3600,(sec/60)%60,sec%60);
   label(mem,timebuf,leftX+leftW/2,299,leftW/2-12,24,RGB(170,198,221),fontBody,DT_RIGHT);
   rounded(mem,leftX,332,leftW-22,9,9,RGB(35,56,76));
   rounded(mem,leftX,332,(leftW-22)*unlocked/PUZZLE_RULES,9,9,blue);
   if(final_stage){
    label(mem,won?"ACCESS GRANTED - YOU WIN":"FINAL CHALLENGE",leftX,361,leftW,26,won?green:RGB(255,191,95),fontTitle,DT_LEFT);
    label(mem,"Original key (read-only; cannot be copied)",leftX,395,leftW,22,RGB(163,186,207),fontSmall,DT_LEFT);
    rounded(mem,leftX,420,leftW-22,originalH,13,RGB(10,23,39));
    RECT rr={leftX+12,430,leftX+leftW-35,420+originalH-10};
    SetBkMode(mem,TRANSPARENT);SetTextColor(mem,RGB(213,238,255));SelectObject(mem,fontBody);
    DrawTextA(mem,original,-1,&rr,DT_WORDBREAK|DT_EDITCONTROL|DT_NOPREFIX);
    label(mem,"Retype your key below (paste disabled)",leftX,originalY+originalH+48,leftW,22,RGB(155,185,206),fontSmall,DT_LEFT);
   }else{
    label(mem,"YOUR PASSWORD",leftX,361,leftW,25,RGB(153,203,225),fontSmall,DT_LEFT);
   }
   label(mem,"AUTO SAVE ENABLED",leftX,cardBottom-122,leftW-22,20,RGB(96,192,160),fontSmall,DT_LEFT);
  }else{
   rounded(mem,leftX,306,leftW-22,86,14,RGB(11,23,40));
   label(mem,"WELCOME, PLAYER",leftX+16,320,leftW-55,29,RGB(233,247,255),fontTitle,DT_LEFT);
   label(mem,"Enter your 11-digit student ID to continue.",leftX+16,355,leftW-54,25,RGB(139,166,191),fontBody,DT_LEFT);
  }
  label(mem,"ACTIVE RULES",rightX,130,rightW-160,29,RGB(237,247,255),fontTitle,DT_LEFT);
  label(mem,"Failures appear first  /  scroll for more",rightX,159,rightW,19,RGB(113,153,182),fontSmall,DT_LEFT);
  if(signed_in){
   const char *str=final_stage?original:password;
   int passed=0,failed[PUZZLE_RULES],success[PUZZLE_RULES],nf=0,ns=0;
   for(int i=0;i<unlocked;i++){if(rule_ok(i,str)){passed++;success[ns++]=i;}else failed[nf++]=i;}
   char counts[65];snprintf(counts,sizeof(counts),"%d PASSED    /    %d FAILED",ns,nf);
   label(mem,counts,rightX,199,rightW-5,24,RGB(149,190,216),fontBody,DT_LEFT);
   rounded(mem,rightX,235,rightW,10,9,RGB(35,56,76));
   if(passed)rounded(mem,rightX,235,rightW*passed/unlocked,10,9,green);
   for(int row=0;row<ruleVisible;row++){
    int rank=row+scroll_pos;if(rank>=unlocked)break;
    int rule=rank<nf?failed[nf-1-rank]:success[ns-1-(rank-nf)];
    int ok=rule_ok(rule,str),y=rulesTop+row*ruleHeight;
    rounded(mem,rightX,y,rightW,ruleHeight-8,13,ok?RGB(18,52,53):RGB(64,34,49));
    rounded(mem,rightX+9,y+9,36,ruleHeight-26,10,ok?RGB(31,103,83):RGB(132,56,64));
    char num[16];snprintf(num,sizeof(num),"%02d",rule+1);
    label(mem,num,rightX+11,y+13,32,23,ok?RGB(126,241,191):RGB(255,173,156),fontBody,DT_CENTER);
    label(mem,names[rule],rightX+54,y+6,rightW-92,ruleHeight-18,RGB(234,242,251),fontBody,DT_LEFT|DT_VCENTER);
    label(mem,ok?"OK":"!",rightX+rightW-34,y+14,25,24,ok?green:red,fontTitle,DT_CENTER);
   }
   if(unlocked>ruleVisible)label(mem,"SCROLL TO EXPLORE MORE RULES",rightX,cardBottom-31,rightW,22,RGB(114,155,185),fontSmall,DT_CENTER);
   if(unlocked>=10&&!final_stage&&!rule_ok(9,password)){
    char cap[64];snprintf(cap,sizeof(cap),"CAPTCHA:  %s",captcha);
    rounded(mem,rightX,cardBottom-82,rightW,39,10,RGB(79,53,37));
    label(mem,cap,rightX+15,cardBottom-71,rightW-30,25,RGB(255,202,121),fontBody,DT_LEFT);
   }
  }else{
   rounded(mem,rightX,220,rightW,125,14,RGB(12,24,41));
   label(mem,"25 RULES + FINAL VERIFICATION",rightX+18,243,rightW-36,33,RGB(82,219,248),fontTitle,DT_LEFT);
   label(mem,"Rules unlock as you solve the puzzle.",rightX+18,296,rightW-35,26,RGB(162,187,207),fontBody,DT_LEFT);
   label(mem,"01   Sign in with student ID",rightX+18,378,rightW-36,26,RGB(138,178,200),fontBody,DT_LEFT);
   label(mem,"02   Construct a valid password",rightX+18,426,rightW-36,26,RGB(138,178,200),fontBody,DT_LEFT);
   label(mem,"03   Re-enter your key to win",rightX+18,474,rightW-36,26,RGB(138,178,200),fontBody,DT_LEFT);
  }
  BitBlt(dc,0,0,W,H,mem,0,0,SRCCOPY);
  SelectObject(mem,oldBmp);DeleteObject(bmp);DeleteDC(mem);
  EndPaint(w,&ps);return 0;
 }
 case WM_DESTROY:if(hPopup)DestroyWindow(hPopup);DeleteObject(fontBody);DeleteObject(fontTitle);DeleteObject(fontSmall);DeleteObject(fontBig);DeleteObject(editBrush);PostQuitMessage(0);return 0;
 }return DefWindowProcA(w,m,wp,lp);}
int WINAPI WinMain(HINSTANCE inst,HINSTANCE prev,LPSTR cmd,int show){(void)prev;(void)cmd;WNDCLASSA c={0};c.lpfnWndProc=wndproc;c.hInstance=inst;c.lpszClassName="PasswordCWinV3";c.hCursor=LoadCursor(NULL,IDC_ARROW);if(!RegisterClassA(&c))return 1;HWND w=CreateWindowA("PasswordCWinV3","Password Game - C Edition",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,1190,750,NULL,NULL,inst,NULL);if(!w)return 1;ShowWindow(w,SW_SHOWMAXIMIZED);MSG msg;while(GetMessageA(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageA(&msg);}return 0;}
#endif
