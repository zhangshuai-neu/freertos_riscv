#include "encoding.h"
#include "headers/bits.h"

//#if __riscv_xlen == 64
//# define STORE    sd
//# define LOAD     ld
//# define REGBYTES 8
//#else
//# define STORE    sw
//# define LOAD     lw
//# define REGBYTES 4
//#endif

#define INT_MASK 0x7FFFFFFF

.section      .text.entry
.align 4

.global portSAVE_CONTEXT
.global portRESTORE_CONTEXT
.global portMTIME_INT
.global portSAVE_RA
.global xPortStartScheduler
.global vPortYield
.global vTaskIncrementTick
.global vPortEndScheduler
.global xExitStack
.global trap_entry

/* Macro for saving task context */
.macro pushREGFILE
	/* make room in stack */
	addi	sp, sp, -REGBYTES * 34

	/* Save Context */
	STORE	x1, 1 * REGBYTES(sp)
	STORE	x2, 2 * REGBYTES(sp)
	//STORE	x3, 3 * REGBYTES(sp)
	STORE	x4, 4 * REGBYTES(sp)
	STORE	x5, 5 * REGBYTES(sp)
	STORE	x6, 6 * REGBYTES(sp)
	STORE	x7, 7 * REGBYTES(sp)
	STORE	x8, 8 * REGBYTES(sp)
	STORE	x9, 9 * REGBYTES(sp)
	STORE	x10, 10 * REGBYTES(sp)
	STORE	x11, 11 * REGBYTES(sp)
	STORE	x12, 12 * REGBYTES(sp)
	STORE	x13, 13 * REGBYTES(sp)
	STORE	x14, 14 * REGBYTES(sp)
	STORE	x15, 15 * REGBYTES(sp)
	STORE	x16, 16 * REGBYTES(sp)
	STORE	x17, 17 * REGBYTES(sp)
	STORE	x18, 18 * REGBYTES(sp)
	STORE	x19, 19 * REGBYTES(sp)
	STORE	x20, 20 * REGBYTES(sp)
	STORE	x21, 21 * REGBYTES(sp)
	STORE	x22, 22 * REGBYTES(sp)
	STORE	x23, 23 * REGBYTES(sp)
	STORE	x24, 24 * REGBYTES(sp)
	STORE	x25, 25 * REGBYTES(sp)
	STORE	x26, 26 * REGBYTES(sp)
	STORE	x27, 27 * REGBYTES(sp)
	STORE	x28, 28 * REGBYTES(sp)
	STORE	x29, 29 * REGBYTES(sp)
	STORE	x30, 30 * REGBYTES(sp)
	STORE	x31, 31 * REGBYTES(sp)
	
.endm

/*saves mstatus and tcb	*/
.macro portSAVE_CONTEXT
	.global	pxCurrentTCB
		/* Store mstatus */
	csrr	t0, mstatus	//pointer
	STORE	t0, 32 * REGBYTES(sp)

	/* Store current stackpointer in task control block (TCB) */
	LOAD	t0, pxCurrentTCB	//pointer
	STORE	sp, 0x0(t0)
.endm


/* Saves current error program counter (EPC) as task program counter */
.macro portSAVE_EPC
  	csrr	t0, mepc
	STORE	t0, 33 * REGBYTES(sp)

.endm

/* Saves current return adress (RA) as task program counter */
.macro portSAVE_RA
	LOAD	t0, 1 * REGBYTES(sp)
	STORE	t0, 33 * REGBYTES(sp)

.endm

//not called on a context switch
.macro portRESTORE_X2
	LOAD 	x2, 2 * REGBYTES(sp)
	//LOAD 	x3, 3 * REGBYTES(sp)
 .endm

 /*restore mstatus and tcb	*/
.macro portRESTORE_CONTEXT
	.global	pxCurrentTCB
	/* Load stack pointer from the current TCB */
	LOAD	sp, pxCurrentTCB
	LOAD	sp, 0x0(sp)
	/* Load task program counter */
	LOAD	t0, 33 * REGBYTES(sp)
  	csrw	mepc, t0
	/* Load saved mstatus */
	LOAD	t0, 32 * REGBYTES(sp)
  	csrw	mstatus, t0
.endm


/* Macro for restoring task context */
.macro popREGFILE

	/* Restore registers,
	Skip global pointer because that does not change */
	LOAD	x1, 1 * REGBYTES(sp)
	//LOAD 	x3, 3 * REGBYTES(sp)
	LOAD	x4, 4 * REGBYTES(sp)
	LOAD	x5, 5 * REGBYTES(sp)
	LOAD	x6, 6 * REGBYTES(sp)
	LOAD	x7, 7 * REGBYTES(sp)
	LOAD	x8, 8 * REGBYTES(sp)
	LOAD	x9, 9 * REGBYTES(sp)
	LOAD	x10, 10 * REGBYTES(sp)
	LOAD	x11, 11 * REGBYTES(sp)
	LOAD	x12, 12 * REGBYTES(sp)
	LOAD	x13, 13 * REGBYTES(sp)
	LOAD	x14, 14 * REGBYTES(sp)
	LOAD	x15, 15 * REGBYTES(sp)
	LOAD	x16, 16 * REGBYTES(sp)
	LOAD	x17, 17 * REGBYTES(sp)
	LOAD	x18, 18 * REGBYTES(sp)
	LOAD	x19, 19 * REGBYTES(sp)
	LOAD	x20, 20 * REGBYTES(sp)
	LOAD	x21, 21 * REGBYTES(sp)
	LOAD	x22, 22 * REGBYTES(sp)
	LOAD	x23, 23 * REGBYTES(sp)
	LOAD	x24, 24 * REGBYTES(sp)
	LOAD	x25, 25 * REGBYTES(sp)
	LOAD	x26, 26 * REGBYTES(sp)
	LOAD	x27, 27 * REGBYTES(sp)
	LOAD	x28, 28 * REGBYTES(sp)
	LOAD	x29, 29 * REGBYTES(sp)
	LOAD	x30, 30 * REGBYTES(sp)
	LOAD	x31, 31 * REGBYTES(sp)

	addi	sp, sp, REGBYTES * 34
.endm

/* Interrupt entry point */
trap_entry:
	/* Check for interrupt */
	pushREGFILE
	csrr	a0, mcause
	blt		a0,zero,interrupt

	/* synchronous interrupt*/
	/* pass sp in a1 */
	mv a1,sp
	jal ulSynchTrap
/*  adjust stack pointer back to where it was prior to ulSynchTrap  */
	mv		sp,a0
	popREGFILE
	mret

/* async interrupt, this function is called */
interrupt:
	li      t0,INT_MASK
	and     a0,a0,t0
	li      t0, 0x7
	beq     a0,t0, MTIME_IRQ
	/* Interupt not m-time interrupt */
	/* pass the exception code in a0 */
	jal 	handle_interrupt
	portRESTORE_X2
	popREGFILE
	mret

	/* Interupt is m-time  */
MTIME_IRQ:
	portSAVE_CONTEXT
	portSAVE_EPC
	jal		vPortSysTickHandler
	portRESTORE_CONTEXT
	popREGFILE
	mret

xPortStartScheduler:
	jal		vPortSetup
	portRESTORE_CONTEXT
	popREGFILE
	mret


vPortYield:
/*  adjust stack pointer back to where it was prior to ulSynchTrap  */
	mv		sp,a0
	portSAVE_CONTEXT

/* context return point was passed back to this function in a1 */
	STORE	a1, 33 * REGBYTES(sp)
    /* switch context */
	jal		vTaskSwitchContext
	portRESTORE_CONTEXT
	popREGFILE
	mret

vPortEndScheduler:
1:
	j		1b


.weak handle_interrupt
handle_interrupt:
1:
  j 1b