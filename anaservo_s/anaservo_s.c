/****************************************************************************/
/* TypeS基板使用マイコンカー　サンプルプログラム Ver1.11                    */
/* オムにホイール使用カメラマシン											*/
/*                          2015.02.25 大阪府立淀川工科高等学校　田辺仁史   */
/****************************************************************************/

/*======================================*/
/* インクルード                         */
/*======================================*/
#include    <no_float.h>                /* stdioの簡略化 最初に置く     */
#include    <stdio.h>                   /* printfなど用                 */
#include    <machine.h>                 /* 組み込み関数用               */
#include    "h8_3048.h"                 /* H8/3048F-ONEレジスタ定義     */
#include    "i2c_eeprom.h"              /* EEP-ROM追加(データ記録)      */
#include    "types_beep.h"              /* TypeS基板のブザー            */
#include 	"isCamera.h"				/* カメラ用ライブラリ			*/
#include 	"isSensor.h"					/* マーカー検出用 */
#include	"drive.h"					/* モータードライブライブラリ	*/
#include 	"sw.h"						/* スイッチライブラリ			*/
#include 	"led.h"						/* LEDライブラリ				*/
#include	"ini.h"						/* 初期設定値					*/
/*======================================*/
/* シンボル定義                         */
/*======================================*/
/* 定数設定 */

#define     FREE                1       /* モータモード　フリー         */
#define     BRAKE               0       /* モータモード　ブレーキ       */

/*======================================*/
/* プロトタイプ宣言                     */
/*======================================*/
void init( void );
void beep_out( int flag );

/*======================================*/
/* グローバル変数の宣言                 */
/*======================================*/

/************************************************************************/
/* メインプログラム                                                     */
/************************************************************************/
void main( void )
{
    int             i, j;
    unsigned int    u;
    char            c;

    /* マイコン機能の初期化 */
    init();                             /* 初期化                       */
    initI2CEeprom( &P6DDR, &P6DR, 0x90, 6, 5);  /* EEP-ROM初期設定      */
    initBeepS();                        /* ブザー関連処理               */
    init_sci1( 0x00, 79 );              /* SCI1初期化                   */
    set_ccr( 0x00 );                    /* 全体割り込み許可             */
	initCamera(10,118);

    /* マイコンカーの状態初期化 */
    motor_mode( BRAKE );
    right_mode( FREE );
	run(0,0);
    LED_Right( 00 );
    setBeepPatternS( 0x8000 );
	pattern = 0;
	pid_flag = 0;

    /* スタート時、スイッチが押されていればデータ転送モード */
    if( pushsw_get() ) {
        pattern = 101;
        cnt1 = 0;
    }

    while( 1 ) {
	sensor_process();
    I2CEepromProcess();                 /* I2C EEP-ROM保存処理          */

    switch( pattern ) {
    case 0:
        /* プッシュスイッチ押下待ち */
        LED_Right( 40 );
        if( pushsw_get() ) {
			Calibration();
            setBeepPatternS( 0x8000 );
            clearI2CEeprom();           /* 数秒かかる                   */
            setBeepPatternS( 0xcc00 );
            cnt1 = 0;
			if(dipsw_get2() == 0x80) pattern = 2000;
            else					 pattern = 1;
            break;
        }
        i =  (cnt1/200) % 2 + 1;
        led_out( i );                   /* LED点滅処理                  */
        break;

    case 1:
        /* スタートバー開待ち */
        LED_Right( 20 );
        led_out( 0x0 );
        cnt1 = 0;
        pattern = 11;
		saveIndex = 0x0000;
        saveFlag = 1;               /* データ保存開始               */
		break;
	case 9:
        LED_Right( 20 );
		speed = 20;pid_flag =1;
		if(R_maker == 1){
		    setBeepPatternS( 0x8000 );
			pattern = 11;
			cnt1 = 0;
		}
		break;
	case 10:
        LED_Right( 20 );
		speed = 20;pid_flag =1;
		if(cnt1 > 1000){
			pattern = 11;
		}
		break;
    case 11:
        /* 通常トレース */
        LED_Right( 20 );
		speed = 30;pid_flag =1;
		if(R_maker == 1){
		    setBeepPatternS( 0x8000 );
			cnt1 = 0;
//			pattern = 12;
//			beep_out(1);
//		}else if(L_maker == 1){
//		    setBeepPatternS( 0xcc00 );
//			beep_out(1);
		}else{
			beep_out(0);
		}
		break;
	case 12:
        LED_Right( 20 );
		speed = 20;pid_flag =1;
		if(cnt1 > 1000) pattern = 13;
		break;
	case 13:
        LED_Right( 0 );
		pid_flag =0;
		motor(0,0,0);
		break;
    case 101:
        /* 停止 */
        LED_Right( 0 );
		motor(0,0,0);
        setBeepPatternS( 0xc000 );
        saveFlag = 0;
        saveSendIndex = 0;
        pattern = 102;
        cnt1 = 0;
        break;

    case 102:
        /* プッシュスイッチが離されたかチェック */
        if( !pushsw_get() ) {
            pattern = 103;
            cnt1 = 0;
        }
        break;

   case 103:
        /* 0.5s待ち */
        if( cnt1 >= 500 ) {
            pattern = 104;
            cnt1 = 0;
        }
        break;

    case 104:
        /* プッシュスイッチが押されたかチェック */
        led_out( cnt1 / 200 % 2 ? 0x6 : 0x9  );
        if( pushsw_get() ) {
            pattern = 105;
            cnt1 = 0;
        }
        break;

    case 105:
        /* タイトル転送、転送準備 */
        printf( "\n" );
        printf( "CarName Data Out\n" ); /* 自分のカーネームを入れてください */
        printf( "Pattern, PID値, 中心値, ライン幅01, ライン幅02, Max, Min, マーカー, " );
        printf( "スピード, 回転\n" );
        pattern = 106;
        break;

    case 106:
        /* データ転送 */
        led_out( 1 << (cnt1/100) % 4 );

        /* 終わりのチェック */
/*        if( (readI2CEeprom( saveSendIndex )==0) ||
                                (saveSendIndex >= 0x8000) ) {
            pattern = 107;
            setBeepPatternS( 0xff00 );
            cnt1 = 0;
            break;
        }
*/
        /* データの転送 */
        printf( "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
            /* パターン */
            (int)readI2CEeprom( saveSendIndex+0 ),
            /* PID値 */
            (int)readI2CEeprom( saveSendIndex+1 ),
            /* 中心値 */
            (int)readI2CEeprom( saveSendIndex+2 ),
            /* ライン幅01 */
            (int)readI2CEeprom( saveSendIndex+3 ),
            /* ライン幅02 */
            (int)readI2CEeprom( saveSendIndex+4 ),
            /* Max */
            (int)readI2CEeprom( saveSendIndex+5 ),
            /* Min */
            (int)readI2CEeprom( saveSendIndex+6 ),
            /* マーカー */
            (int)readI2CEeprom( saveSendIndex+7 )

        );

        saveSendIndex += 16;            /* 次の送信準備                 */
        break;

    case 107:
        /* 転送終了 */
        led_out( 0xf );
        break;

	case 2000:
		if(cnt0 > 500){
			if(R_maker == 1){
			    setBeepPatternS( 0x8000 );
				beep_out(1);
			}else if(L_maker == 1){
			    setBeepPatternS( 0xcc00 );
				beep_out(1);
			}else{
				beep_out(0);
			}
//			raw_view();
			bi_view();
			cnt0 = 0;
		}
		break;
	
        default:
            break;
        }
    }

}
/************************************************************************/
/* ブザー制御                                                           */
/* 引数　0:ブザーOFF 1:ブザーON                                         */
/************************************************************************/
void beep_out( int flag )
{
    if( flag ) {
        P6DR |= 0x10;
    } else {
        P6DR &= 0xef;
    }
}
/************************************************************************/
/* end of file                                                          */
/************************************************************************/