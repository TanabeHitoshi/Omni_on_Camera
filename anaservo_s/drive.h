#ifndef	DRIVE_H
#define	DRIVE_H

#define     MOTOR_CYCLE         3072    /* モータPWMのサイクル兼、      */
                                        /*          タイマ値 1ms        */
#define     MOTOR_OFFSET        37      /* パルスをずらす時間           */
                                        /* 1=325.52[ns]                 */
/* 現在の状態保存用 */
extern signed int   speed;          /* 現在のスピード      */
extern int			pid_flag;		/* ＰＩＤ制御を行うか */

/* プロトタイプ宣言*/
void speed_r( int accele_l, int accele_r );
void speed2_r( int accele_l, int accele_r );
void speed_f( int accele_l, int accele_r );
void speed2_f( int accele_l, int accele_r );
void motor_mode_r( int mode_l, int mode_r );
void motor_mode_f( int mode_l, int mode_r );
void motor_mode( int mode );
                     
void motor( int accele_l, int accele_m,int accele_r );
void run( int sp, int turn );

#endif