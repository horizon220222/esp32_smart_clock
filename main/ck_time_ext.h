#ifndef CK_TIME_EXT_H
#define CK_TIME_EXT_H

#define UART_NUM UART_NUM_0
#define BUF_SIZE (1024)


// 倒计时函数声明
void countdown_task(void);
// 开场动画函数声明
void opening_animation(void);
// 时间同步任务函数声明
void time_sync_task(void *pv);
// 串口监听任务函数声明
void uart_listen_task(void *pv);

#endif
