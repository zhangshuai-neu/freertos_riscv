/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM CM3 port.
 * 定义在
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"

#include "platform.h"
#include "encoding.h"

/* Standard Includes */
#include <stdlib.h>
#include <unistd.h>


/* Each task maintains its own interrupt status in the critical nesting
variable. 
每个任务维持自己中断嵌套变量
*/
UBaseType_t uxCriticalNesting = 0xaaaaaaaa;

#if USER_MODE_TASKS
	unsigned long MSTATUS_INIT = (MSTATUS_MPIE);
#else
	unsigned long MSTATUS_INIT = (MSTATUS_MPP | MSTATUS_MPIE);
#endif


/*
 * Used to catch tasks that attempt to return from their implementing function.
 * 捕捉任务返回
 */
static void prvTaskExitError( void );


/*-----------------------------------------------------------*/

/* System Call Trap */
// 系统调用陷阱
//ECALL macro stores argument in a2， ECALL宏，在a2中存放参数
unsigned long ulSynchTrap(unsigned long mcause, unsigned long sp, unsigned long arg1)	{

	switch(mcause)	{
		//on User and Machine ECALL, handler the request
		case 8:
		case 11:
			if(arg1==IRQ_DISABLE)	{
				//zero out mstatus.mpie
				clear_csr(mstatus,MSTATUS_MPIE);

			} else if(arg1==IRQ_ENABLE)	{
				//set mstatus.mpie
				set_csr(mstatus,MSTATUS_MPIE);

			} else if(arg1==PORT_YIELD)		{
				//always yield from machine mode
				//fix up mepc on sync trap
				unsigned long epc = read_csr(mepc);
				vPortYield(sp,epc+4); //never returns
			} else if(arg1==PORT_YIELD_TO_RA)	{

				vPortYield(sp,(*(unsigned long*)(sp+1*sizeof(sp)))); //never returns
			}

			break;

		default:
			write(1, "trap\n", 5);
			_exit(mcause);
	}

	//fix mepc and return
	unsigned long epc = read_csr(mepc);
	write_csr(mepc,epc+4);
	return sp;
}

// 进入临界区，此处仅仅是关中断
void vPortEnterCritical( void )
{
	#if USER_MODE_TASKS
		//e200停用
		ECALL(IRQ_DISABLE);
	#else
		portDISABLE_INTERRUPTS();
	#endif

	uxCriticalNesting++;
}
/*-----------------------------------------------------------*/
// 离开临界区，此处仅仅是开中断
void vPortExitCritical( void )
{
	configASSERT( uxCriticalNesting );
	uxCriticalNesting--;
	if( uxCriticalNesting == 0 )
	{
		#if USER_MODE_TASKS
			ECALL(IRQ_ENABLE);
		#else
			portENABLE_INTERRUPTS();
		#endif
	}

	return;
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

/* Clear current interrupt mask and set given mask */
void vPortClearInterruptMask(int mask)
{
	write_csr(mie,mask);
}
/*-----------------------------------------------------------*/

/* Set interrupt mask and return current interrupt enable register */
int xPortSetInterruptMask()
{
	uint32_t ret;
	ret = read_csr(mie);
	write_csr(mie,0);
	return ret;
}

/*-----------------------------------------------------------*/
/*
 * See header file for description.
 * 设置栈寄存器
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
	/* Simulate the stack frame as it would be created by a context switch
	interrupt. 
	建立模拟的栈框架，手动创建栈的上下文。
	每个任务都是从中断进入任务切换的
	*/

	register int *tp asm("x3");
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE)pxCode;			/* Start address */

	//set the initial mstatus value
	pxTopOfStack--;
	*pxTopOfStack = MSTATUS_INIT;

	pxTopOfStack -= 22;
	*pxTopOfStack = (portSTACK_TYPE)pvParameters;	/* Register a0 */
	//pxTopOfStack -= 7;
	//*pxTopOfStack = (portSTACK_TYPE)tp; /* Register thread pointer */
	//pxTopOfStack -= 2;
	pxTopOfStack -=9;
	*pxTopOfStack = (portSTACK_TYPE)prvTaskExitError; /* Register ra */
	pxTopOfStack--;

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/


void prvTaskExitError( void )
{
	/* A function that implements a task must not exit or attempt to return to
	its caller as there is nothing to return to.  If a task wants to exit it
	should instead call vTaskDelete( NULL ).
	Artificially force an assert() to be triggered if configASSERT() is
	defined, then stop here so application writers can catch the error. */
	configASSERT( uxCriticalNesting == ~0UL );
	portDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/


/*Entry Point for Machine Timer Interrupt Handler*/
void vPortSysTickHandler(){
	static uint64_t then = 0;

	clear_csr(mie, MIP_MTIP);
    volatile uint64_t * mtime       = (uint64_t*) (CLINT_CTRL_ADDR + CLINT_MTIME);
    volatile uint64_t * mtimecmp    = (uint64_t*) (CLINT_CTRL_ADDR + CLINT_MTIMECMP);

	if(then != 0)  {
		//next timer irq is 1 second from previous
		then += (configRTC_CLOCK_HZ / configTICK_RATE_HZ);
	} else{ //first time setting the timer
		uint64_t now = *mtime;
		then = now + (configRTC_CLOCK_HZ / configTICK_RATE_HZ);
	}
	*mtimecmp = then;


	/* Increment the RTOS tick. */
	if( xTaskIncrementTick() != pdFALSE )
	{
		vTaskSwitchContext();
	}
	set_csr(mie, MIP_MTIP);
}
/*-----------------------------------------------------------*/


// 配置时钟
void vPortSetupTimer()	{

    // Set the machine timer
	// 设置机器时间
    volatile uint64_t * mtime       = (uint64_t*) (CLINT_CTRL_ADDR + CLINT_MTIME);
    volatile uint64_t * mtimecmp    = (uint64_t*) (CLINT_CTRL_ADDR + CLINT_MTIMECMP);
    uint64_t now = *mtime;
    uint64_t then = now + (configRTC_CLOCK_HZ / configTICK_RATE_HZ);
    *mtimecmp = then;

    // Enable the Machine-Timer bit in MIE
	// 打开时钟
    set_csr(mie, MIP_MTIP);
}
/*-----------------------------------------------------------*/


void vPortSetup()	{

	vPortSetupTimer();
	uxCriticalNesting = 0;
}
/*-----------------------------------------------------------*/















