/*======================================*/
/* インクルード                         */
/*======================================*/
#include <stdio.h>
#include "h8_3048.h"                  /* R8C/38A SFRの定義ファイル    */
#include <stdio.h>                 /* printf使用ライブラリ         */
#include "isCamera.h"
#include "drive.h"

/* TAOS TSL1401CL */
#define		TAOS_SI_HIGH	P8DR |= 0x02	/* Port P8_1 bit */
#define		TAOS_SI_LOW		P8DR &= 0xfd	/* Port P8_1 bit */
#define		TAOS_CLK_HIGH	P8DR |= 0x01	/* Port P8_0 bit */
#define		TAOS_CLK_LOW	P8DR &= 0xfe	/* Port P8_0 bit */
#define		TAOS_EXPOSURE_DELAY	expose()

#define White_min	9					/* 白色の最小値 			*/
#define	White_Max	150					/* ライン白色MAX値の設定 */

int 			ImageData[128];			/* カメラの値				*/
int				BinarizationData[128];	/* ２値化					*/
int   			EXPOSURE_timer = 0;		/* 露光時間					*/
int 			LineStart,LineStop;		/* 読み取り位置の始めと終わり */
int				Max,Min,Ave;			/*カメラ読み取り最大値、最小値、平均値*/
unsigned int 	Rsensor01;				/* ラインの右端 */
unsigned int 	Lsensor01;				/* ラインの左端 */
unsigned int 	Wide01;					/* ラインの幅 */
unsigned int 	Rsensor02;				/* ラインの右端 */
unsigned int 	Lsensor02;				/* ラインの左端 */
unsigned int 	Wide02;					/* ラインの幅 */
unsigned int 	Rsensor;				/* ラインの右端 */
unsigned int 	Lsensor;				/* ラインの左端 */
//unsigned int 	Wide;					/* ラインの幅 */
int				R_maker;				/* 左マーカー */
int				L_maker;				/* 右マーカー */
int			 	Center;					/* ラインの重心 */
int			 	Center01;				/* ラインの重心 */
int			 	Center02;				/* ラインの重心 */
int 			White;					/* 白色の個数	*/
int				pid_turn;

/************************************************************************/
/* センサー読み取り                                             */
/* 引数　 なし                                                          */
/* 戻り値 なし			                                                */
/************************************************************************/
void sensor_process(void) {
	
	int i;
	int s;
	
	ImageCapture();
	expose();
	ImageCapture();				/* イメージキャプチャ 	*/
	binarization();				/* ２値化				*/
	WhiteLineWide();			/* 白線の幅を測定		*/
	WhiteLineCenter();			/* 白線の中心を測定		*/

//	printf("Exposure = %d Max = %d Min = %d Ave = %d Wide = %d Center = %d\n",EXPOSURE_timer,Max,Min,Ave,WhiteLineWide(),WhiteLineCenter());
	pid_turn = PID();
}
/************************************************************************/
/* カメラの初期化                                                */
/* 引数　 開始位置、終了位置                                                          */
/* 戻り値 なし                                                 */
/************************************************************************/
void initCamera(int Start,int Stop)
{
	LineStart = Start;
	LineStop = Stop;
}
/************************************************************************/
/* イメージキャプチャ                                                 */
/* 引数　 開始位置、終了位置                                                          */
/* 戻り値 なし                                                 */
/************************************************************************/
void ImageCapture(void){	 
	
	unsigned char i;

	Max = 0,Min = 1024;

	TAOS_SI_HIGH;  
	TAOS_CLK_HIGH;  
	TAOS_SI_LOW;
	ImageData[0] = 0;//get_ad7();	// inputs data from camera (first pixel)
	TAOS_CLK_LOW;

	for(i = 1; i < LineStart; i++) {		
		TAOS_CLK_HIGH;		
		TAOS_CLK_LOW;
	}
	for(i = LineStart; i < LineStop; i++) {					 
		TAOS_CLK_HIGH;
		ImageData[i] = get_ad2();	// inputs data from camera (one pixel each time through loop) 
		TAOS_CLK_LOW;
		
		if(Max < ImageData[i]){
			Max = ImageData[i];
		}			
		if(Min > ImageData[i]){
			Min = ImageData[i];
		}	
	}
	for(i = LineStop; i < 128; i++) {		
		TAOS_CLK_HIGH;		
		TAOS_CLK_LOW;
	}

	TAOS_CLK_HIGH;
	TAOS_CLK_LOW;
}
/************************************************************************/
/* ２値化                                                               */
/* 引数　 なし                                                          */
/* 戻り値 なし			                                                */
/************************************************************************/
void binarization(void)
{
	int i;

	/* 最高値と最低値から間の値を求める */
	Ave = (Max + Min) * 2 / 3;
	/* 黒は０　白は１にする */
	White = 0;					/* 白の数を０にする */
	if( Max > 50 ){
		/* 白が一直線のとき */
		if(Min > 80){
			White = 128;
			for(i=1; i < 128; i++){
				BinarizationData[i] = 1;
			}
		}else{		
			for(i = LineStart ; i < LineStop; i++) {
				if(  ImageData[i] > Ave ){	
					White++;			
					BinarizationData[i] = 1;
				}else{
					BinarizationData[i] = 0;
				}
			}
		}
	/* 黒が一面のとき */
	}else{
		for(i=1; i < 128; i++){
			BinarizationData[i] = 0;
		}
	}

}
/************************************************************************/
/* 白線の幅を測定                                                       */
/* 引数　 なし                                                          */
/* 戻り値 なし			                                                 */
/************************************************************************/
void WhiteLineWide(void)
{
	int t=0,i;
		
	Lsensor01 = Lsensor02 = LineStart;
	Rsensor01 = Rsensor02 = LineStop;
		
	for(i = Lsensor01 ; i < LineStop; i++) {
		if(t==0){	/* １本目の白線 */
			if( BinarizationData[i] ){		/* 左から最初の白 */
				Lsensor01 = i;
				t = 1;
			}
		}else if(t==1){
			if( BinarizationData[i] ){		/* 左から最初の白 */
				Lsensor01 = i;
				t = 2;
			}
		}else if(t==2){	/* １本目の黒線 */
			if( !BinarizationData[i] ){		/* 左から最初の黒 */			
				Rsensor01 = i;
				t = 3;
			}
		}else if(t==3){
			if( !BinarizationData[i] ){		/* 左から最初の黒 */			
				Rsensor01 = i;
				t = 4;
			}
		}else if(t==4){	/* ２本目の白線 */
			if( BinarizationData[i] ){		/* 左から最初の白 */
				Lsensor02 = i;
				t = 5;
			}
		}else if(t==5){
			if( BinarizationData[i] ){		/* 左から最初の白 */
				Lsensor02 = i;
				t = 6;
			}
		}else if(t==6){	/* ２本目の黒線 */
			if( !BinarizationData[i] ){		/* 左から最初の黒 */			
				Rsensor02 = i;
				t = 7;
			}
		}else if(t==7){	
			if( !BinarizationData[i] ){		/* 左から最初の黒 */			
				Rsensor02 = i;
				break;
			}
		}
	}
	if(White > White_min){
		Wide01 = Rsensor01 - Lsensor01;		/* 1本目の白線幅を求める */	
		if( Lsensor02 != LineStart){
			Wide02 = Rsensor02 - Lsensor02; /* 2本目の白線幅を求める */
		}else{
			Wide02 = 0;						/* 2本目白線なし		*/
		}
	}else{
		Wide01 = Wide02 = 0;				/* 黒一面 */
	}			
}
/************************************************************************/
/* 白線の中心を測定                                                       */
/* 引数　 なし                                                          */
/* 戻り値 なし			                                                 */
/************************************************************************/
void WhiteLineCenter(void)
{
	int abs_Center01,abs_Center02;		/* センター値の絶対値 */

	if(White > White_min){
		if(Wide01 != 0){
			Center01 = (Lsensor01 + Rsensor01)/2 - 64;	/* 重心を求める */	
		}
		if(Wide02 != 0){
			Center02 = (Lsensor02 + Rsensor02)/2 - 64;	/* 重心を求める */
		}			
	}else{
		Center = 0;								/* 黒一面 */
	}
	 
	/* マーカーの判別 */
	if( Wide01 != 0 ){
		if( Wide02 == 0){			/* 白線が１本のとき */
			Center = Center01;
			R_maker = 0; L_maker = 0;
		}else if( Wide01 > 20){		/* Wide01がマーカー　Wide02が白線 */
			Center = Center02;
			L_maker = 1; R_maker = 0; 
		}else if( Wide02 > 20){		/* Wide01が白線　Wide02がマーカー */
			Center = Center01;
			L_maker = 0; R_maker = 1; 
		}else{					
			/* センター値の絶対値 */
			if(Center01 > 0) abs_Center01 = Center01;
			else			 abs_Center01 = -Center01;
			if(Center02 > 0) abs_Center02 = Center02;
			else			 abs_Center02 = -Center02;
			if( abs_Center01 > abs_Center02){/* Wide01がマーカー　Wide02が白線 */
				Center = Center02;
				L_maker = 1; R_maker = 0; 
			}else if( abs_Center01 < abs_Center02){/* Wide01が白線　Wide02がマーカー */
				Center = Center01;
				L_maker = 0; R_maker = 1; 
			}
		}
	}	
}
/************************************************************************/
/* キャリブレーション                                                   */
/* 引数　 なし                                                          */
/* 戻り値 なし			                                                */
/************************************************************************/
void Calibration(void)
{
	int i;
	printf("Calibration now......\n");
	while(Max < White_Max){
		ImageCapture();
		for(i=0; i < EXPOSURE_timer ;i++);
		ImageCapture();
		EXPOSURE_timer += 50;
	}
	printf("EXPOSURE_timer = %d\n",EXPOSURE_timer);
}
/************************************************************************/
/* 露光時間調整                                                         */
/* 引数　 なし                                                          */
/* 戻り値 なし　　　　　                                                */
/************************************************************************/
void expose( void )
{
	int i;
	
//	if( Wide != 0 && !(White >= 90)){//黒でなく白でもない
//printf("White_Max = %d Max = %d\n",White_Max,Max);
//		if(Max < White_Max){
//			EXPOSURE_timer += (White_Max - Max)*10;
//			EXPOSURE_timer += 10;
//		}else{
//			EXPOSURE_timer -= (Max - White_Max)*10;
//			EXPOSURE_timer -= 10;
//		}
//		EXPOSURE_timer += (White_Max - Max);
//	}
//	if( EXPOSURE_timer > 1000) EXPOSURE_timer = 1000;
//	if( EXPOSURE_timer < 0 ) EXPOSURE_timer = 0;
	
	for(i=0;i<EXPOSURE_timer;i++);

}
/************************************************************************/
/* RAWデータの表示	                                                   */
/* 引数　 なし                                                          */
/* 戻り値 なし			                                                 */
/************************************************************************/
void raw_view(void)
{
	int i;
	for(i = LineStart;i < LineStop; i++){
		printf("%d ",ImageData[i]);
	}
	printf("\n\n");
}
/************************************************************************/
/* ２値化データの表示	                                                   */
/* 引数　 なし                                                          */
/* 戻り値 なし			                                                 */
/************************************************************************/
void bi_view(void)
{
	int i;
	for(i = LineStart;i < LineStop; i++){
		if(BinarizationData[i] == 1){
			printf("*");
		}else{
			printf(" ");
		}
	}
	printf("Exposure = %d Max = %d Min = %d Ave = %d Wide = %d Center = %d\n",EXPOSURE_timer,Max,Min,Ave,Wide01,Center);
	printf("Lsensor01 = %d Rsensor01 = %d Wide01 = %d Lsensor02 = %d Rsensor02 = %d Wide02 = %d L_maker = %d R_maker = %d\n",
			Lsensor01,Rsensor01,Wide01,Lsensor02,Rsensor02,Wide02,L_maker,R_maker);
}

/************************************************************************/
/* PID				                                                    */
/* 引数　 なし                                                          */
/* 戻り値 なし			                                                */
/************************************************************************/
int PID(void)
{
	static float	iCenter = 0.0;
	static float	preCenter = 0.0;
	float			h;
//	Center /= 2;
	iCenter +=  (float)Center - preCenter;
	h = (float)Center * Kp + iCenter * Ki + ((float)Center - preCenter) * Kd;
	preCenter = (float)Center;
	
	return h;
}

/************************************************************************/
/* A/D値読み込み(AN2)                                                   */
/* 引数　 なし                                                          */
/* 戻り値 A/D値 0～1023                                                 */
/************************************************************************/
int get_ad2( void )
{
    int i;

    /* A/Dコンバータの設定 */
	i = i;						/* φADの1サイクルウエイト入れる*/
	AD_CSR |= 0x20;				/* AD スタート */
	while( !(AD_CSR & 0x80) );	/* エンドフラグをチェック */
	AD_CSR &= 0x7f;				/* エンドフラグクリア */
	i = AD_DRC >> 6;			/* 代入 */
	return i;
}
