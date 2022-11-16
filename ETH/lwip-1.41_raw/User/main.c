/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* SPDX-License-Identifier: Apache-2.0
*******************************************************************************/

#include "debug.h"
#include "Jubei_eth_driver.h"
#include "lwip/tcp.h"
#include "string.h"
/* Global typedef */

/* Global define */

/* Global Variable */

static void echo_client_init(void);

//当连接失败后， 调用echo_client_init重新建立客户端，重新开始连接
static void echo_client_conn_err(void *arg,err_t err)
{
    printf("connect error! closed by core!!\n");
    printf("try to connect to sever again!!\n");
    echo_client_init();
}

//当接收到服务器的数据后，这个函数被内核调用，p中包含了服务器数据
static err_t echo_client_recv(void *arg,struct tcp_pcb *pcb,struct pbuf *p,err_t err)
{
    if(p!=NULL)
    {
        tcp_recved(pcb, p->tot_len);     //更新接收窗口
        tcp_write(pcb, p->payload, p->len, 1);   //回显服务器的数据
        pbuf_free(p);
    }else if(err == ERR_OK)   //如果服务器断开了连接
    {
        tcp_close(pcb);      //先断开到服务器的连接
        echo_client_init();  //再次建立新的连接
        return ERR_OK;
    }
    return ERR_OK;
}

//当客户端向服务器发送数据成功后，这个函数被内核调用
static err_t echo_client_sent(void *arg,struct tcp_pcb *pcb,uint16_t len)
{
    printf("echo client send data OK! sent len = %d\n",len);
    return ERR_OK;
}

//当客户端同服务器连接成功后，这个函数被内核调用
static err_t echo_client_connected(void *arg,struct tcp_pcb *pcb,err_t err)
{
    char GREENTING[] = "Hi,I am a new Client!\n";

    tcp_recv(pcb,echo_client_recv);        //注册recv函数
    tcp_sent(pcb,echo_client_sent);        //注册sent函数
    tcp_write(pcb,GREENTING,sizeof(GREENTING),1);     //向服务器发送GREETING
    return ERR_OK;
}

//客户端初始化函数，需要在内核初始化后调用一次
static void echo_client_init(void)
{
    struct tcp_pcb *pcb = NULL;
    struct ip_addr server_ip;
    pcb = tcp_new();
    IP4_ADDR(&server_ip,192,168,1,11);
    tcp_bind(pcb, IP_ADDR_ANY, 20);
    tcp_connect(pcb,&server_ip,5,echo_client_connected);  //连接服务器21端口
    tcp_err(pcb,echo_client_conn_err);                    //注册连接错误时的回调函数
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Delay_Init();
	USART_Printf_Init(115200);

	printf("SystemClk:%d\r\n",SystemCoreClock);

	printf("This is printf example\r\n");
	LwIP_Init();
    echo_client_init();

	while(1)
    {
     sys_check_timeouts();
	}
}

