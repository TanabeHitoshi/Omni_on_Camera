#ifndef	ISCAMERA_H
#define	ISCAMERA_H

#define Kp 0.7
#define Ki 0.2
#define Kd 0.3

extern int 				ImageData[128];
extern int 				sensor8;
extern unsigned int 	Wide01;					/* ラインの幅 */
extern unsigned int 	Wide02;					/* ラインの幅 */
extern int			 	Center;					/* ラインの重心 */
extern int				pid_turn;				
extern int			 	Max;					/* 最大値 */
extern int			 	Min;					/* 最小値 */
extern int				R_maker;				/* 左マーカー */
extern int				L_maker;				/* 右マーカー */

/* プロトタイプ宣言 */
void			sensor_process(void);			/* センサー読み取り		*/
void			initCamera(int,int);			/* カメラの初期化		*/
void			ImageCapture(void);				/* イメージキャプチャ 	*/
void			binarization(void);				/* ２値化				*/
void 			WhiteLineWide(void);			/* 白線の幅を測定		*/
void 			WhiteLineCenter(void);			/* 白線の中心を測定		*/
void 			Calibration(void);				/* キャリブレーション	*/
void 			expose( void );					/* 露光時間調整 		*/
void			raw_view(void);					/* RAWデータの表示		*/
void			bi_view(void);					/* ２値化データの表示	*/
unsigned char	sensor_inp(unsigned char);		/* センサ状態検出		*/
int 			PID(void);						/* PID					*/
int				get_ad2( void );				/* A/D値読み込み(AN2)	*/

#endif