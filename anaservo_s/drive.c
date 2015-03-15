/*======================================*/
/* インクルード                         */
/*======================================*/
#include <stdio.h>
#include "h8_3048.h"                  /* R8C/38A SFRの定義ファイル    */
#include <stdio.h>                 /* printf使用ライブラリ         */
#include "i2c_eeprom.h"              /* EEP-ROM追加(データ記録)      */
#include "isCamera.h"
#include "drive.h"
#include "sw.h"						/* スイッチライブラリ			*/


/* 現在の状態保存用 */
int         speed;          /* 現在のスピード      */
int			pid_flag;		/* ＰＩＤ制御を行うか */

/************************************************************************/
/* 後輪の速度制御                                                       */
/* 引数　 左モータ:-100～100 , 右モータ:-100～100                       */
/*        0で停止、100で正転100%、-100で逆転100%                        */
/************************************************************************/
void speed_r( int accele_l, int accele_r )
{
    unsigned int    sw_data;
    unsigned int    work;

    sw_data  = dipsw_get() + 5;         /* ディップスイッチ読み込み     */
    sw_data *= 5;                       /* 5～20 → 25～100に変換       */

    /* 左モータ */
    if( accele_l > 0 ) {
        PBDR &= 0xbf;
    } else if( accele_l < 0 ) {
        PBDR |= 0x40;
        accele_l = -accele_l;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET - 2;
    work   = (long)work * accele_l * sw_data / 10000;
    ITU4_BRA = MOTOR_CYCLE / 2 - 2 - work;

    /* 右モータ */
    if( accele_r > 0 ) {
        PBDR &= 0x7f;
    } else if( accele_r < 0 ) {
        PBDR |= 0x80;
        accele_r = -accele_r;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET - 2;
    work   = (long)work * accele_r * sw_data / 10000;
    ITU3_BRB = MOTOR_CYCLE / 2 - 2 - work;
}

/************************************************************************/
/* 後輪の速度制御2 ディップスイッチには関係しないspeed関数              */
/* 引数　 左モータ:-100～100 , 右モータ:-100～100                       */
/*        0で停止、100で正転100%、-100で逆転100%                        */
/************************************************************************/
void speed2_r( int accele_l, int accele_r )
{
    unsigned int    sw_data;
    unsigned int    work;

    /* 左モータ */
    if( accele_l > 0 ) {
        PBDR &= 0xbf;
    } else if( accele_l < 0 ) {
        PBDR |= 0x40;
        accele_l = -accele_l;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET - 2;
    work   = (long)work * accele_l / 100;
    ITU4_BRA = MOTOR_CYCLE / 2 - 2 - work;

    /* 右モータ */
    if( accele_r > 0 ) {
        PBDR &= 0x7f;
    } else if( accele_r < 0 ) {
        PBDR |= 0x80;
        accele_r = -accele_r;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET - 2;
    work   = (long)work * accele_r / 100;
    ITU3_BRB = MOTOR_CYCLE / 2 - 2 - work;
}

/************************************************************************/
/* 前輪の速度制御                                                       */
/* 引数　 左モータ:-100～100 , 右モータ:-100～100                       */
/*        0で停止、100で正転100%、-100で逆転100%                        */
/************************************************************************/
void speed_f( int accele_l, int accele_r )
{
    unsigned char   sw_data;
    unsigned long   speed_max;

    sw_data  = dipsw_get() + 5;         /* ディップスイッチ読み込み     */
    speed_max = (unsigned long)(MOTOR_CYCLE-1) * sw_data / 20;

    /* 左前モータ */
    if( accele_l > 0 ) {
        PADR &= 0xfd;
    } else if( accele_l < 0 ) {
        PADR |= 0x02;
        accele_l = -accele_l;
    }
    /* GRBがCNTより20以上小さい値かどうかのチェック */
    if( ITU0_GRB > 20 ) {
        /* GRBが20以上なら　単純に比較 */
        while( (ITU0_CNT >= ITU0_GRB-20) && (ITU0_CNT <= ITU0_GRB) );
    } else {
        /* GRBが20以下なら　上限値からの値も参照する */
        while( (ITU0_CNT >= ITU0_GRA-20) || (ITU0_CNT <= ITU0_GRB) );
    }
    ITU0_GRB = speed_max * accele_l / 100;

    /* 右前モータ */
    if( accele_r > 0 ) {
        PADR &= 0xbf;
    } else if( accele_r < 0 ) {
        PADR |= 0x40;
        accele_r = -accele_r;
    }
    /* GRBがCNTより20以上小さい値かどうかのチェック */
    if( ITU1_GRB > 20 ) {
        /* GRBが20以上なら　単純に比較 */
        while( (ITU1_CNT >= ITU1_GRB-20) && (ITU1_CNT <= ITU1_GRB) );
    } else {
        /* GRBが20以下なら　上限値からの値も参照する */
        while( (ITU1_CNT >= ITU1_GRA-20) || (ITU1_CNT <= ITU1_GRB) );
    }
    ITU1_GRB = speed_max * accele_r / 100;
}

/************************************************************************/
/* 前輪の速度制御2 ディップスイッチには関係しないspeed関数              */
/* 引数　 左モータ:-100～100 , 右モータ:-100～100                       */
/*        0で停止、100で正転100%、-100で逆転100%                        */
/************************************************************************/
void speed2_f( int accele_l, int accele_r )
{
    unsigned long   speed_max;


    speed_max = MOTOR_CYCLE - 1;

    /* 左前モータ */
    if( accele_l > 0 ) {
        PADR &= 0xfd;
    } else if( accele_l < 0 ) {
        PADR |= 0x02;
        accele_l = -accele_l;
    }
    /* GRBがCNTより20以上小さい値かどうかのチェック */
    if( ITU0_GRB > 20 ) {
        /* GRBが20以上なら　単純に比較 */
        while( (ITU0_CNT >= ITU0_GRB-20) && (ITU0_CNT <= ITU0_GRB) );
    } else {
        /* GRBが20以下なら　上限値からの値も参照する */
        while( (ITU0_CNT >= ITU0_GRA-20) || (ITU0_CNT <= ITU0_GRB) );
    }
    ITU0_GRB = speed_max * accele_l / 100;

    /* 右前モータ */
    if( accele_r > 0 ) {
        PADR &= 0xbf;
    } else if( accele_r < 0 ) {
        PADR |= 0x40;
        accele_r = -accele_r;
    }
    /* GRBがCNTより20以上小さい値かどうかのチェック */
    if( ITU1_GRB > 20 ) {
        /* GRBが20以上なら　単純に比較 */
        while( (ITU1_CNT >= ITU1_GRB-20) && (ITU1_CNT <= ITU1_GRB) );
    } else {
        /* GRBが20以下なら　上限値からの値も参照する */
        while( (ITU1_CNT >= ITU1_GRA-20) || (ITU1_CNT <= ITU1_GRB) );
    }
    ITU1_GRB = speed_max * accele_r / 100;
}

/************************************************************************/
/* 後モータ停止動作（ブレーキ、フリー）                                 */
/* 引数　 左モータ:FREE or BRAKE , 右モータ:FREE or BRAKE               */
/************************************************************************/
void motor_mode_r( int mode_l, int mode_r )
{
    if( mode_l ) {
        P1DR |= 0x01;
    } else {
        P1DR &= 0xfe;
    }
    if( mode_r ) {
        P1DR |= 0x02;
    } else {
        P1DR &= 0xfd;
    }
}

/************************************************************************/
/* 前モータ停止動作（ブレーキ、フリー）                                 */
/* 引数　 左モータ:FREE or BRAKE , 右モータ:FREE or BRAKE               */
/************************************************************************/
void motor_mode_f( int mode_l, int mode_r )
{
    if( mode_l ) {
        P1DR |= 0x04;
    } else {
        P1DR &= 0xfb;
    }
    if( mode_r ) {
        P1DR |= 0x08;
    } else {
        P1DR &= 0xf7;
    }
}
/************************************************************************/
/* モータ停止動作（ブレーキ、フリー）	                                */
/* 引数　 FREE or BRAKE 									            */
/************************************************************************/
void motor_mode( int mode )
{
    motor_mode_f( mode, mode );
    motor_mode_r( mode, mode );
}

/************************************************************************/
/* モータ速度制御                                                       */
/* 引数　 左モータ:-100～100、後モータ:-100～100、右モータ:-100～100    */
/*        0で停止、100で正転100%、-100で逆転100%                        */
/* 戻り値 なし                                                          */
/************************************************************************/
void motor( int accele_l, int accele_m,int accele_r )
{
	speed_r(accele_r,accele_m);
	speed_f(0,accele_l);
}
/************************************************************************/
/* 走行				                                                    */
/* 引数　 走行:-100～100、0で停止、100で正転100%、-100で逆転100% 		*/
/*        回転:-100～100、「+」で時計方向、「－」で反時計方向           */
/* 戻り値 なし                                                          */
/************************************************************************/
void run( int sp, int turn )
{
	int	sp_l,sp_m,sp_r;

    saveData[8] = sp;             /* ログ保存                     */
    saveData[9] = turn;             /* ログ保存                     */

	/* 左モーター */
	sp_l = sp + turn;
	if(sp_l > 100) sp_l = 100;
	if(sp_l < -100) sp_l = -100;
	/* 後モーター */
	sp_m = -turn;
	if(sp_m > 100) sp_m = 100;
	if(sp_m < -100) sp_m = -100;
	/* 右モーター */
	sp_r = sp - turn; 
	if(sp_r > 100) sp_r = 100;
	if(sp_r < -100) sp_r = -100;
  

	motor(sp_l,sp_m,sp_r);
}
/************************************************************************/
/* end of file                                                          */
/************************************************************************/