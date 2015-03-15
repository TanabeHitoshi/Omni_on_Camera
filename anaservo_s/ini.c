/*======================================*/
/* インクルード                         */
/*======================================*/
#include <stdio.h>
#include "h8_3048.h"                  /* R8C/38A SFRの定義ファイル    */
#include <stdio.h>                 	/* printf使用ライブラリ         */
#include "i2c_eeprom.h"       	   /* EEP-ROM追加(データ記録)      */
#include "isCamera.h"
#include "drive.h"
#include "sw.h"						/* スイッチライブラリ			*/
#include "ini.h"

unsigned long   cnt0;                   /* timer関数用                  */
unsigned long   cnt1;                   /* main内で使用                 */
unsigned long   cnt_lcd;                /* LCD処理で使用                */
unsigned long   stop_timer;				/* 走行タイマー					*/

int             pattern;                /* パターン番号                 */
int             iTimer10;               /* 10msカウント用               */
int				Cycle;					/* 周期							*/

unsigned long	cnt_wdt;				/* WDT用						*/
int				cnt_wdt2;				/* WDT内部処理用				*/
/************************************************************************/
/* H8/3048F-ONE 内蔵周辺機能の初期化                                    */
/************************************************************************/
void init( void )
{
    /* ポートの入出力設定 */
    P1DDR = 0xff;
    P2DDR = 0x00;
    P3DDR = 0xff;
    P4DDR = 0xff;
    P5DDR = 0xff;
    P6DDR = 0x90;                       /* CPU基板上のDIP SW            */
    P8DDR = 0xe3;						/* P80 -> CLK   P81 -> SI		*/
    P9DDR = 0xf7;
    PADDR = 0xd6;
    PBDDR = 0xff;

    /* A/Dの初期設定 */
    AD_CSR = 0x0a;                      /* スキャンモード使用AN2    CKS -> 134 */
//    AD_CSR |= 0x20;                     /* ADスタート                   */

    /* ITU0 左前モータ用PWM */
    ITU0_TCR = 0x23;                    /* カウンタ、クリアの設定       */
    ITU0_GRA = MOTOR_CYCLE;             /* PWM周期                      */
    ITU0_GRB = 0;                       /* デューティ比設定             */

    /* ITU1 右前モータ用PWM */
    ITU1_TCR = 0x23;                    /* カウンタ、クリアの設定       */
    ITU1_GRA = MOTOR_CYCLE;             /* PWM周期                      */
    ITU1_GRB = 0;                       /* デューティ比設定             */

    /* ITU2 エンコーダ */
    ITU2_TCR = 0x14;                    /* PA0パルス入力端子            */

    /* ITU3,4 右後、左後、サーボモータ用PWM兼、割り込み */
    ITU3_TCR = 0x03;
    ITU4_TCR = 0x03;
    ITU_FCR  = 0x2e;                    /* ITU3,4で相補PWMモード        */
    ITU_TOCR = 0x12;                    /* TIOCB3,TOCXA4,TOCXB4は反転   */
    ITU3_IER = 0x01;                    /* TCNT = GRAによる割込み制御   */
    ITU3_CNT = MOTOR_OFFSET;
    ITU4_CNT = 0;
    ITU3_GRA = MOTOR_CYCLE / 2 + MOTOR_OFFSET - 2;  /* PWM周期設定      */
    ITU3_BRB = ITU3_GRB = MOTOR_CYCLE / 2 - 2;
    ITU4_BRA = ITU4_GRA = MOTOR_CYCLE / 2 - 2;
    ITU4_BRB = ITU4_GRB = MOTOR_CYCLE / 2 - 2;

    ITU_MDR = 0x03;                     /* PWMモード設定                */
    ITU_STR = 0x1f;                     /* ITUのカウントスタート        */
	
	/* WDT設定	*/
	TCSR_W = 0xa521;					/* WDT=インターバルタイマ */
	cnt_wdt = 0;
	cnt_wdt2 =0;

}

/************************************************************************/
/* ITU3 割り込み処理                                                    */
/************************************************************************/
#pragma interrupt( interrupt_timer3 )
void interrupt_timer3( void )
{
    unsigned int    i;

    ITU3_TSR &= 0xfe;                   /* フラグクリア                 */
    cnt1++;
	cnt0++;

    /* ブザー処理 */
    beepProcessS();

	/* PID走行 */
	Cycle--;
	if( (pid_flag == 1) && (Cycle < 0) ){
		run(speed,pid_turn);
		Cycle = 4;
	}

	/* データ保存関連 */
    iTimer10++;
    if( iTimer10 >= 10 ) {
        iTimer10 = 0;
        if( saveFlag ) {
            saveData[0] = pattern;      /* パターン                     */
            saveData[1] = pid_turn;		/* PID値   */
            saveData[2] = Center;		/* 中心値 */
            saveData[3] = Wide01;		/* ライン幅01 */
            saveData[4] = Wide02;		/* ライン幅02 */
            saveData[5] = Max;			/* 最大値 */
            saveData[6] = Min;			/* 最小値 */
            saveData[7] = L_maker << 4 | R_maker;	/* マーカー */
            /*  8はモータ関数内でスピード保存    */
            /*  9はモータ関数内で回転値保存    */
            saveData[10] = 0;    
            saveData[11] = 0;    
            saveData[12] = 0;
            saveData[13] = 0;
            saveData[14] = 0;
            saveData[15] = 0;
            setPageWriteI2CEeprom( saveIndex, 16, saveData );
            saveIndex += 16;
            if( saveIndex >= 0x8000 ) saveFlag = 0;
        }
    }
}
/************************************************************************/
/* WDT 割り込み処理                                                    */
/************************************************************************/
#pragma interrupt( interrupt_wdt )
void interrupt_wdt( void )
{
	unsigned char work;

	work = TCSR & 0x7f;			/* フラグリード */
	TCSR_W = 0xa500 | work;		/* フラグクリア */

	cnt_wdt++;
}
/************************************************************************/
/* タイマ本体                                                           */
/* 引数　 タイマ値 1=1ms                                                */
/************************************************************************/
void timer( unsigned long timer_set )
{
    cnt0 = 0;
    while( cnt0 < timer_set );
}
/************************************************************************/
/* end of file                                                          */
/************************************************************************/