/****************************************************************************/
/* モータドライブ基板TypeS テストプログラム Ver.1.01                        */
/*                          2007.12 ジャパンマイコンカーラリー実行委員会    */
/****************************************************************************/

/*======================================*/
/* インクルード                         */
/*======================================*/
#include    <no_float.h>                /* stdioの簡略化 最初に置く     */
#include    <stdio.h>                   /* printfなど用                 */
#include    <machine.h>                 /* 組み込み関数用               */
#include    "h8_3048.h"                 /* H8/3048F-ONEレジスタ定義     */
#include    "i2c_eeprom.h"              /* EEP-ROM追加(データ記録)      */

/*======================================*/
/* シンボル定義                         */
/*======================================*/

/* 定数設定 */
#define     MOTOR_CYCLE         3072    /* モータPWMのサイクル兼、      */
                                        /*          タイマ値 1ms        */
#define     MOTOR_OFFSET        37      /* パルスをずらす時間           */
                                        /* 1=325.52[ns]                 */

#define     FREE                1       /* モータモード　フリー         */
#define     BRAKE               0       /* モータモード　ブレーキ       */

/*======================================*/
/* プロトタイプ宣言                     */
/*======================================*/
void init( void );
unsigned char sensor_inp( void );
unsigned char center_inp( void );
unsigned char startbar_get( void );
unsigned char dipsw_get( void );
unsigned char dipsw_get2( void );
unsigned char pushsw_get( void );
unsigned char cn8_get( void );
void led_out( unsigned char led );
void speed_r( int accele_l, int accele_r );
void speed2_r( int accele_l, int accele_r );
void speed_f( int accele_l, int accele_r );
void speed2_f( int accele_l, int accele_r );
void motor_mode_r( int mode_l, int mode_r );
void motor_mode_f( int mode_l, int mode_r );
void servoPwmOut( int pwm );
void motor_mode_s( int mode );
void beep_out( int flag );

/*======================================*/
/* グローバル変数の宣言                 */
/*======================================*/
int             pattern;                /* マイコンカー動作パターン     */
unsigned long   cnt1;                   /* タイマ用                     */
int             iPWM_l, iPWM_r, iPWM_s; /* 相補PWM設定バッファ用        */

/* エンコーダ関連 */
int             iTimer10;               /* 10msカウント用               */
long            lEncoderTotal;          /* 積算値保存用                 */
int             iEncoderMax;            /* 10ms毎の値の最大値保存用     */
int             iEncoder;               /* 10ms毎の最新値               */
unsigned int    uEncoderBuff;           /* 計算用　割り込み内で使用     */

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
    init_sci1( 0x00, 79 );              /* SCI1初期化                   */
    set_ccr( 0x00 );                    /* 全体割り込み許可             */

    /* マイコンカーの状態初期化 */
    speed_r( 0, 0 );
    speed_f( 0, 0 );
    servoPwmOut( 0 );

    while( 1 ) {
        switch( pattern ) {
        case 0:
            /* メニュー */
            printf( "\n\n" );
            printf( "Motor Drive PCB TypeS Test Program Ver1.00\n" );
            printf( "\n" );
            printf( "1 : LEDのテスト\n" );
            printf( "2 : スイッチのテスト\n" );
            printf( "3 : CN8の入力テスト\n" );
            printf( "4 : ブザーのテスト\n" );
            printf( "5 : エンコーダのテスト\n" );
            printf( "6 : EEP-ROMのテスト\n" );
            printf( "7 : ボリュームのテスト\n" );
            printf( "8 : アナログセンサ基板のテスト\n" );
            printf( "9 : モータのテスト\n" );
            printf( "\n" );
            printf( "1-9の数字を入力してください " );
            pattern = 1;
            break;

        case 1:
            if( get_sci(&c) ) {
                if( c >= '1' && c <= '9' ) {
                    printf( "%c\n\n" , c );
                    cnt1 = 0;
                    pattern = (c - 0x30) * 10 + 1;
                }
            }
            break;

        case 11:
            /* LEDのテスト */
            printf( "現在LED2～5を順番に点灯中です。" );
            printf( "終わったらどれかキーを押してください。\n" );
            pattern = 12;
            break;

        case 12:
            led_out( 1 << (cnt1 / 500) );
            if( cnt1 >= 2000 ) cnt1 = 0;
            if( get_sci(&c) == 1 ) {
                led_out( 0 );
                pattern = 0;
            }
            break;

        case 21:
            /* スイッチのテスト */
            printf( "現在スイッチの値を表示中です。" );
            printf( "終わったらどれかキーを押してください。\n" );
            pattern = 22;
            break;

        case 22:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                break;
            }
            if( cnt1 >= 200 ) {
                cnt1 = 0;
                printf( "ディップSW(SW3)の値 : %02x ", dipsw_get2() );
                printf( "プッシュSW(SW2)の値 : %1x\r", pushsw_get() );
            }
            break;

        case 31:
            /* CN8の入力テスト */
            printf( "CN8に入力されている値を表示中です。" );
            printf( "終わったらどれかキーを押してください。\n" );
            pattern = 32;
            break;

        case 32:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                break;
            }
            if( cnt1 >= 200 ) {
                cnt1 = 0;
                printf( "CN8の入力値 : %1x\r", cn8_get() );
            }
            break;

        case 41:
            /* ブザーのテスト */
            printf( "ブザーをテストします。" );
            printf( "終わったらどれかキーを押してください。\n" );
            pattern = 42;
            break;

        case 42:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                beep_out( 0 );
                break;
            }
            if( cnt1 <= 1000 ) {
                beep_out( 1 );
            } else if( cnt1 <= 2000 ) {
                beep_out( 0 );
            } else {
                cnt1 = 0;
            }
            break;

        case 51:
            /* エンコーダのテスト */
            printf( "エンコーダの入力パルス数を表示します。" );
            printf( "終わったらどれかキーを押してください。\n" );
            lEncoderTotal = 0;
            pattern = 52;
            break;

        case 52:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                break;
            }
            if( cnt1 >= 200 ) {
                cnt1 = 0;
                printf( "エンコーダのパルス値 : %8ld\r", lEncoderTotal );
            }
            break;

        case 61:
            /* EEP-ROMのテスト */
            printf( "EEP-ROMが正常に書き込めるかチェックします。" );
            printf( "チェック中はキー入力を受け付けません。" );
            printf( "チェックしますか？(Y or N) " );
            pattern = 62;
            break;

        case 62:
            if( get_sci(&c) == 1 ) {
                switch( c ) {
                case 'Y':
                case 'y':
                    printf( "y\n" );
                    pattern = 63;
                    break;
                case 'N':
                case 'n':
                    printf( "n\n" );
                    pattern = 0;
                    break;
                }
            }
            break;

        case 63:
            for( u=0; u<0x8000 ; u++ ) {
                if( u % 0x80 == 0 ) {
                    printf( "%3ld%%チェック完了...\r",
                                        (long)u * 100 / 0x8000 );
                }
                writeI2CEeprom( u, 0xaa );
                while( !checkI2CEeprom() );
                if( (unsigned char)readI2CEeprom( u ) != 0xaa ) {
                    break;
                }

                writeI2CEeprom( u, 0x55 );
                while( !checkI2CEeprom() );
                if( (unsigned char)readI2CEeprom( u ) != 0x55 ) {
                    break;
                }

                writeI2CEeprom( u, 0x00 );
                while( !checkI2CEeprom() );
                if( (unsigned char)readI2CEeprom( u ) != 0x00 ) {
                    break;
                }
            }
            printf( "\n" );
            if( u == 0x8000 ) {
                printf( "EEP-ROMは正常でした。\n" );
           } else {
                printf( "EEP-ROMに異常がありました。" );
                printf( "半田不良など確認してください。\n" );
            }
            pattern = 64;
            break;

        case 64:
            printf( "どれかキーを押してください。" );
            while( get_sci(&c) );
            pattern = 65;
            break;

        case 65:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                break;
            }
            break;

        case 71:
            /* ボリュームのテスト */
            printf( "ボリューム電圧を表示中です。" );
            printf( "終わったらどれかキーを押してください。\n" );
            pattern = 72;
            break;

        case 72:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                break;
            }
            if( cnt1 >= 200 ) {
                cnt1 = 0;
                i =  AD_DRB >> 6;
                j = (long)500 * i / 1023;
                printf( "ボリュームの電圧 : %4d (%01d.%02dV)\r",
                                                i, j/100, j%100 );
            }
            break;

        case 81:
            /* アナログセンサ基板のテスト */
            printf( "アナログセンサ基板の値を表示中です。" );
            printf( "終わったらどれかキーを押してください。\n" );
            pattern = 82;
            break;

        case 82:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                break;
            }
            if( cnt1 >= 200 ) {
                cnt1 = 0;
                printf( "Left=%4d , Right=%4d , "
                        "Digital=%1x , Center=%1d , Bar=%1d\r",
                            AD_DRD>>6, AD_DRC>>6, sensor_inp(),
                            center_inp(), startbar_get() );
            }
            break;

        case 91:
            /* モータのテスト */
            printf( "モータのテストをします。\n" );
            printf( "1 : 左後ろモータ\n" );
            printf( "2 : 右後ろモータ\n" );
            printf( "3 : サーボモータ\n" );
            printf( "4 : 左前モータ\n" );
            printf( "5 : 右前モータ\n" );
            printf( "0 : 終了\n" );
            printf( "\n" );
            printf( "0-5の数字を入力してください " );
            pattern = 92;
            break;

        case 92:
            if( get_sci(&c) ) {
                if( c == '0' ) {
                    pattern = 0;
                    break;
                } else if( c >= '1' && c <= '5' ) {
                    printf( "%c\n\n" , c );
                    cnt1 = 0;
                    pattern = (c - 0x31) * 10 + 101;
                }
            }
            break;

        case 101:
            printf( "左後ろモータをテストします。\n" );
            printf( "スペースキーを押して動作を変えます。" );
            printf( "0キーで終了です。\n" );
            cnt1 = 0;
            i = 0;
            j = 1;
            pattern = 102;
            break;

        case 102:
            if( get_sci(&c) ) {
                if( c == '0' ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                    pattern = 91;
                    break;
                } else if( c == ' ' ) {
                    cnt1 = 0;
                    i++;
                    if( i >= 5 ) i = 0;
                }
            }
            switch( i ) {
            case 0:
                if( i != j ) {
                    printf( "停止中                     \r" );
                    j = i;
                }
                speed2_r( 0, 0 );
                motor_mode_r( BRAKE , BRAKE );
                break;
            case 1:
                if( i != j ) {
                    printf( "正転50%% <-> ブレーキ動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( 50, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 2:
                if( i != j ) {
                    printf( "逆転50%% <-> ブレーキ動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( -50, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 3:
                if( i != j ) {
                    printf( "正転50%% <-> フリー　動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( 50, 0 );
                    motor_mode_r( FREE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( FREE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 4:
                if( i != j ) {
                    printf( "逆転50%% <-> フリー　動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( -50, 0 );
                    motor_mode_r( FREE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( FREE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            }
            break;

        case 111:
            printf( "右後ろモータをテストします。\n" );
            printf( "スペースキーを押して動作を変えます。" );
            printf( "0キーで終了です。\n" );
            cnt1 = 0;
            i = 0;
            j = 1;
            pattern = 112;
            break;

        case 112:
            if( get_sci(&c) ) {
                if( c == '0' ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                    pattern = 91;
                    break;
                } else if( c == ' ' ) {
                    cnt1 = 0;
                    i++;
                    if( i >= 5 ) i = 0;
                }
            }
            switch( i ) {
            case 0:
                if( i != j ) {
                    printf( "停止中                     \r" );
                    j = i;
                }
                speed2_r( 0, 0 );
                motor_mode_r( BRAKE , BRAKE );
                break;
            case 1:
                if( i != j ) {
                    printf( "正転50%% <-> ブレーキ動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( 0, 50 );
                    motor_mode_r( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 2:
                if( i != j ) {
                    printf( "逆転50%% <-> ブレーキ動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( 0, -50 );
                    motor_mode_r( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 3:
                if( i != j ) {
                    printf( "正転50%% <-> フリー　動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( 0, 50 );
                    motor_mode_r( BRAKE , FREE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , FREE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 4:
                if( i != j ) {
                    printf( "逆転50%% <-> フリー　動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( 0, -50 );
                    motor_mode_r( BRAKE , FREE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , FREE );
                } else {
                    cnt1 = 0;
                }
                break;
            }
            break;

        case 121:
            printf( "サーボモータをテストします。\n" );
            printf( "スペースキーを押して動作を変えます。" );
            printf( "0キーで終了です。\n" );
            cnt1 = 0;
            i = 0;
            j = 1;
            pattern = 122;
            break;

        case 122:
            if( get_sci(&c) ) {
                if( c == '0' ) {
                    servoPwmOut( 0 );
                    motor_mode_s( BRAKE );
                    pattern = 91;
                    break;
                } else if( c == ' ' ) {
                    cnt1 = 0;
                    i++;
                    if( i >= 5 ) i = 0;
                }
            }
            switch( i ) {
            case 0:
                if( i != j ) {
                    printf( "停止中                     \r" );
                    j = i;
                }
                servoPwmOut( 0 );
                motor_mode_s( BRAKE );
                break;
            case 1:
                if( i != j ) {
                    printf( "正転50%% <-> ブレーキ動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    servoPwmOut( 50 );
                    motor_mode_s( BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    servoPwmOut( 0 );
                    motor_mode_s( BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 2:
                if( i != j ) {
                    printf( "逆転50%% <-> ブレーキ動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    servoPwmOut( -50 );
                    motor_mode_s( BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    servoPwmOut( 0 );
                    motor_mode_s( BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 3:
                if( i != j ) {
                    printf( "正転50%% <-> フリー　動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    servoPwmOut( 50 );
                    motor_mode_s( FREE );
                } else if(  cnt1 <= 2000 ) {
                    servoPwmOut( 0 );
                    motor_mode_s( FREE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 4:
                if( i != j ) {
                    printf( "逆転50%% <-> フリー　動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    servoPwmOut( -50 );
                    motor_mode_s( FREE );
                } else if(  cnt1 <= 2000 ) {
                    servoPwmOut( 0 );
                    motor_mode_s( FREE );
                } else {
                    cnt1 = 0;
                }
                break;
            }
            break;

        case 131:
            printf( "左前モータをテストします。\n" );
            printf( "スペースキーを押して動作を変えます。" );
            printf( "0キーで終了です。\n" );
            cnt1 = 0;
            i = 0;
            j = 1;
            pattern = 132;
            break;

        case 132:
            if( get_sci(&c) ) {
                if( c == '0' ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                    pattern = 91;
                    break;
                } else if( c == ' ' ) {
                    cnt1 = 0;
                    i++;
                    if( i >= 5 ) i = 0;
                }
            }
            switch( i ) {
            case 0:
                if( i != j ) {
                    printf( "停止中                     \r" );
                    j = i;
                }
                speed2_f( 0, 0 );
                motor_mode_f( BRAKE , BRAKE );
                break;
            case 1:
                if( i != j ) {
                    printf( "正転50%% <-> ブレーキ動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( 50, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 2:
                if( i != j ) {
                    printf( "逆転50%% <-> ブレーキ動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( -50, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 3:
                if( i != j ) {
                    printf( "正転50%% <-> フリー　動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( 50, 0 );
                    motor_mode_f( FREE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( FREE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 4:
                if( i != j ) {
                    printf( "逆転50%% <-> フリー　動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( -50, 0 );
                    motor_mode_f( FREE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( FREE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            }
            break;

        case 141:
            printf( "右後ろモータをテストします。\n" );
            printf( "スペースキーを押して動作を変えます。" );
            printf( "0キーで終了です。\n" );
            cnt1 = 0;
            i = 0;
            j = 1;
            pattern = 142;
            break;

        case 142:
            if( get_sci(&c) ) {
                if( c == '0' ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                    pattern = 91;
                    break;
                } else if( c == ' ' ) {
                    cnt1 = 0;
                    i++;
                    if( i >= 5 ) i = 0;
                }
            }
            switch( i ) {
            case 0:
                if( i != j ) {
                    printf( "停止中                     \r" );
                    j = i;
                }
                speed2_f( 0, 0 );
                motor_mode_f( BRAKE , BRAKE );
                break;
            case 1:
                if( i != j ) {
                    printf( "正転50%% <-> ブレーキ動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( 0, 50 );
                    motor_mode_f( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 2:
                if( i != j ) {
                    printf( "逆転50%% <-> ブレーキ動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( 0, -50 );
                    motor_mode_f( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 3:
                if( i != j ) {
                    printf( "正転50%% <-> フリー　動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( 0, 50 );
                    motor_mode_f( BRAKE , FREE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , FREE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 4:
                if( i != j ) {
                    printf( "逆転50%% <-> フリー　動作中\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( 0, -50 );
                    motor_mode_f( BRAKE , FREE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , FREE );
                } else {
                    cnt1 = 0;
                }
                break;
            }
            break;

        default:
            break;
        }
    }
}

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
    P8DDR = 0xe0;
    P9DDR = 0xf7;
    PADDR = 0xd6;
    PBDDR = 0xff;

    /* A/Dの初期設定 */
    AD_CSR = 0x1b;                      /* スキャンモード使用AN0-AN3    */
    AD_CSR |= 0x20;                     /* ADスタート                   */

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

    /* ITU3,4 右、左モータ、ステアリング */
    ITU3_TCR = 0x03;
    ITU4_TCR = 0x03;
    ITU_FCR  = 0x2e;                    /* ITU3,4で相補PWMモード        */
    ITU_TOCR = 0x12;                    /* TIOCB3,TOCXA4,TOCXB4は反転   */
    ITU3_IER = 0x01;                    /* TCNT = GRAによる割込み制御   */
    ITU3_CNT = MOTOR_OFFSET;
    ITU4_CNT = 0;
    ITU3_GRA = MOTOR_CYCLE / 2 + MOTOR_OFFSET - 1;  /* PWM周期設定      */
    ITU3_BRB = ITU3_GRB = iPWM_r = MOTOR_CYCLE / 2;
    ITU4_BRA = ITU4_GRA = iPWM_l = MOTOR_CYCLE / 2;
    ITU4_BRB = ITU4_GRB = iPWM_s = MOTOR_CYCLE / 2;

    ITU_MDR = 0x03;                     /* PWMモード設定                */
    ITU_STR = 0x1f;                     /* ITUのカウントスタート        */
}

/************************************************************************/
/* ITU3 割り込み処理                                                    */
/************************************************************************/
#pragma interrupt( interrupt_timer3 )
void interrupt_timer3( void )
{
    unsigned int    i;

    ITU3_TSR &= 0xfe;                   /* フラグクリア                 */
    ITU3_BRB = iPWM_r;                  /* PWM値の更新                  */
    ITU4_BRA = iPWM_l;                  /* PWM値の更新                  */
    ITU4_BRB = iPWM_s;                  /* PWM値の更新                  */
    cnt1++;

    /* エンコーダ制御 */
    iTimer10++;
    if( iTimer10 >= 10 ) {
        iTimer10 = 0;
        i = ITU2_CNT;
        iEncoder       = i - uEncoderBuff;
        lEncoderTotal += iEncoder;
        if( iEncoder > iEncoderMax )
                    iEncoderMax = iEncoder;
        uEncoderBuff = i;
    }
}

/************************************************************************/
/* センサ状態検出                                                       */
/* 引数　 なし                                                          */
/* 戻り値 センサ値                                                      */
/************************************************************************/
unsigned char sensor_inp( void )
{
    unsigned char sensor;

    sensor  = (~PADR & 0x08) | (~P8DR & 0x07);

    return sensor;
}

/************************************************************************/
/* 中心センサ状態                                                       */
/* 引数　 なし                                                          */
/* 戻り値 センサ値                                                      */
/************************************************************************/
unsigned char center_inp( void )
{
    unsigned char sensor;

    sensor  = ~PADR & 0x20;
    sensor  = !!sensor;

    return sensor;
}

/************************************************************************/
/* スタートバーセンサ状態                                               */
/* 引数　 なし                                                          */
/* 戻り値 センサ値                                                      */
/************************************************************************/
unsigned char startbar_get( void )
{
    unsigned char sensor;

    sensor  = ~P8DR & 0x08;
    sensor  = !!sensor;

    return sensor;
}

/************************************************************************/
/* ディップスイッチ値読み込み                                           */
/* 戻り値 スイッチ値 0～15                                              */
/************************************************************************/
unsigned char dipsw_get( void )
{
    unsigned char sw;

    sw  = ~P6DR;                        /* ディップスイッチ読み込み     */
    sw &= 0x0f;

    return  sw;
}

/************************************************************************/
/* ディップスイッチ値読み込み                                           */
/* 戻り値 スイッチ値 0～15                                              */
/************************************************************************/
unsigned char dipsw_get2( void )
{
    unsigned char sw;

    sw  = ~P2DR;                        /* モータドライブ基板のsw読み込み*/

    return  sw;
}

/************************************************************************/
/* プッシュスイッチ値読み込み                                           */
/* 戻り値 スイッチ値 ON:1 OFF:0                                         */
/************************************************************************/
unsigned char pushsw_get( void )
{
    unsigned char sw;

    sw = ~P8DR & 0x10;                  /* プッシュスイッチ読み込み     */
    sw = !!sw;

    return  sw;
}

/************************************************************************/
/* プッシュスイッチ値読み込み                                           */
/* 戻り値 スイッチ値 ON:1 OFF:0                                         */
/************************************************************************/
unsigned char cn8_get( void )
{
    unsigned char data;

    data = P7DR >> 4;

    return  data;
}

/************************************************************************/
/* LED制御                                                              */
/* 引数　4個のLED制御 0～15                                             */
/************************************************************************/
void led_out( unsigned char led )
{
    unsigned char data;

    led <<= 4;
    data = P1DR & 0x0f;
    P1DR = data | led;
}

/************************************************************************/
/* 後輪の速度制御                                                       */
/* 引数　 左モータ:-100～100 , 右モータ:-100～100                       */
/*        0で停止、100で正転100%、-100で逆転100%                        */
/************************************************************************/
void speed_r( int accele_l, int accele_r )
{
    unsigned int    sw_data;
    unsigned int    work;

    sw_data  = dipsw_get() + 5;             /* ディップスイッチ読み込み */
    sw_data *= 5;                           /* 5～20 → 25～100に変換   */

    /* 左モータ */
    if( accele_l > 0 ) {
        PBDR &= 0xbf;
    } else if( accele_l < 0 ) {
        PBDR |= 0x40;
        accele_l = -accele_l;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET;
    work   = (long)work * accele_l * sw_data / 10000;
    iPWM_l = MOTOR_CYCLE / 2 - work;

    /* 右モータ */
    if( accele_r > 0 ) {
        PBDR &= 0x7f;
    } else if( accele_r < 0 ) {
        PBDR |= 0x80;
        accele_r = -accele_r;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET;
    work   = (long)work * accele_r * sw_data / 10000;
    iPWM_r = MOTOR_CYCLE / 2 - work;
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
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET;
    work   = (long)work * accele_l / 100;
    iPWM_l = MOTOR_CYCLE / 2 - work;

    /* 右モータ */
    if( accele_r > 0 ) {
        PBDR &= 0x7f;
    } else if( accele_r < 0 ) {
        PBDR |= 0x80;
        accele_r = -accele_r;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET;
    work   = (long)work * accele_r / 100;
    iPWM_r = MOTOR_CYCLE / 2 - work;
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

    sw_data  = dipsw_get() + 5;             /* ディップスイッチ読み込み */
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
/* 後モータ動作（ブレーキ、フリー）                                     */
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
/* 前モータ動作（ブレーキ、フリー）                                     */
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
/* サーボモータ制御                                                     */
/* 引数　 サーボモータPWM：-100～100                                    */
/************************************************************************/
void servoPwmOut( int pwm )
{
    unsigned int    work;

    if( pwm > 0 ) {
        PADR &= 0x7f;
    } else if( pwm < 0 ) {
        PADR |= 0x80;
        pwm = -pwm;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET;
    work   = (long)work * pwm / 100;
    iPWM_s = MOTOR_CYCLE / 2 - work;
}

/************************************************************************/
/* サーボモータ動作（ブレーキ、フリー）                                 */
/* 引数　 FREE or BRAKE                                                 */
/************************************************************************/
void motor_mode_s( int mode )
{
    if( mode ) {
        P3DR |= 0x80;
    } else {
        P3DR &= 0x7f;
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