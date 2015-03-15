#ifndef	ISCAMERA_H
#define	ISCAMERA_H

#define Kp 0.7
#define Ki 0.2
#define Kd 0.3

extern int 				ImageData[128];
extern int 				sensor8;
extern unsigned int 	Wide01;					/* ���C���̕� */
extern unsigned int 	Wide02;					/* ���C���̕� */
extern int			 	Center;					/* ���C���̏d�S */
extern int				pid_turn;				
extern int			 	Max;					/* �ő�l */
extern int			 	Min;					/* �ŏ��l */
extern int				R_maker;				/* ���}�[�J�[ */
extern int				L_maker;				/* �E�}�[�J�[ */

/* �v���g�^�C�v�錾 */
void			sensor_process(void);			/* �Z���T�[�ǂݎ��		*/
void			initCamera(int,int);			/* �J�����̏�����		*/
void			ImageCapture(void);				/* �C���[�W�L���v�`�� 	*/
void			binarization(void);				/* �Q�l��				*/
void 			WhiteLineWide(void);			/* �����̕��𑪒�		*/
void 			WhiteLineCenter(void);			/* �����̒��S�𑪒�		*/
void 			Calibration(void);				/* �L�����u���[�V����	*/
void 			expose( void );					/* �I�����Ԓ��� 		*/
void			raw_view(void);					/* RAW�f�[�^�̕\��		*/
void			bi_view(void);					/* �Q�l���f�[�^�̕\��	*/
unsigned char	sensor_inp(unsigned char);		/* �Z���T��Ԍ��o		*/
int 			PID(void);						/* PID					*/
int				get_ad2( void );				/* A/D�l�ǂݍ���(AN2)	*/

#endif