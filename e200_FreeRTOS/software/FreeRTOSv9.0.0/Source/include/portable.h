/*-----------------------------------------------------------
 * Portable layer API.  Each function must be defined for each port.
 * 端口层API 每个函数都需要在各自的port.c中实现
 *----------------------------------------------------------*/

#ifndef PORTABLE_H
#define PORTABLE_H

/* Each FreeRTOS port has a unique portmacro.h header file.  Originally a
pre-processor definition was used to ensure the pre-processor found the correct
portmacro.h file for the port being used.  That scheme was deprecated in favour
of setting the compiler's include path such that it found the correct
portmacro.h file - removing the need for the constant and allowing the
portmacro.h file to be located anywhere in relation to the port being used.
Purely for reasons of backward compatibility the old method is still valid, but
to make it clear that new projects should not use it, support for the port
specific constants has been moved into the deprecated_definitions.h header
file. */
/*
每个FreeRTOS端口都有一个唯一的portmacro.h头文件， portmacro.h 通常和port.c放在一起。
旧版本的定义方式放在deprecated_definitions.h 头文件
*/

#include "deprecated_definitions.h"

/* If portENTER_CRITICAL is not defined then including deprecated_definitions.h
did not result in a portmacro.h header file being included - and it should be
included here.  In this case the path to the correct portmacro.h header file
must be set in the compiler's include path. 

如果未定义portENTER_CRITICAL，则包括deprecated_definitions.h不会导致
包含portmacro.h头文件 - 并且应该包含在此处。  在这种情况下，必须在编译器
的include路径中设置正确的portmacro.h头文件的路径
*/
#ifndef portENTER_CRITICAL
	#include "portmacro.h"
#endif

#if portBYTE_ALIGNMENT == 32
	#define portBYTE_ALIGNMENT_MASK ( 0x001f )
#endif

#if portBYTE_ALIGNMENT == 16
	#define portBYTE_ALIGNMENT_MASK ( 0x000f )
#endif

#if portBYTE_ALIGNMENT == 8
	#define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#endif 

#if portBYTE_ALIGNMENT == 4
	#define portBYTE_ALIGNMENT_MASK	( 0x0003 )
#endif

#if portBYTE_ALIGNMENT == 2
	#define portBYTE_ALIGNMENT_MASK	( 0x0001 )
#endif

#if portBYTE_ALIGNMENT == 1
	#define portBYTE_ALIGNMENT_MASK	( 0x0000 )
#endif

#ifndef portBYTE_ALIGNMENT_MASK
	#error "Invalid portBYTE_ALIGNMENT definition"
#endif

#ifndef portNUM_CONFIGURABLE_REGIONS
	#define portNUM_CONFIGURABLE_REGIONS 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "mpu_wrappers.h"

/*
 * Setup the stack of a new task so it is ready to be placed under the
 * scheduler control.  The registers have to be placed on the stack in
 * the order that the port expects to find them.
 * 
 * 配置新任务的栈，在调度器的控制下。
 * 寄存器
 */
#if( portUSING_MPU_WRAPPERS == 1 )
	StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters, BaseType_t xRunPrivileged ) PRIVILEGED_FUNCTION;
#else
	StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters ) PRIVILEGED_FUNCTION;
#endif

/* Used by heap_5.c. */
typedef struct HeapRegion
{
	uint8_t *pucStartAddress;
	size_t xSizeInBytes;
} HeapRegion_t;

// 定义堆管理的区域
// 该函数必须由 pvPortMalloc()调用、
// 创建任务、队列、信号量、互斥锁、软件中断等内容时，将会调用pvPortMalloc
// 具体的内存管理方法，可以看MemBang下heap*.c文件
void vPortDefineHeapRegions( const HeapRegion_t * const pxHeapRegions ) PRIVILEGED_FUNCTION;

// 映射到内存管理例程的接口
void *pvPortMalloc( size_t xSize ) PRIVILEGED_FUNCTION;
void vPortFree( void *pv ) PRIVILEGED_FUNCTION;
void vPortInitialiseBlocks( void ) PRIVILEGED_FUNCTION;
size_t xPortGetFreeHeapSize( void ) PRIVILEGED_FUNCTION;
size_t xPortGetMinimumEverFreeHeapSize( void ) PRIVILEGED_FUNCTION;

// 配置硬件为调度器做准备，通常配置时钟中断和设置正确的时钟频率
// portasm.S
BaseType_t xPortStartScheduler( void ) PRIVILEGED_FUNCTION;

// 撤销为调度器准备的硬件环境
// portasm.S
void vPortEndScheduler( void ) PRIVILEGED_FUNCTION;

/*
 * The structures and methods of manipulating the MPU are contained within the
 * port layer.
 *
 * Fills the xMPUSettings structure with the memory region information
 * contained in xRegions.
 */
#if( portUSING_MPU_WRAPPERS == 1 )
	struct xMEMORY_REGION;
	void vPortStoreTaskMPUSettings( xMPU_SETTINGS *xMPUSettings, const struct xMEMORY_REGION * const xRegions, StackType_t *pxBottomOfStack, uint32_t ulStackDepth ) PRIVILEGED_FUNCTION;
#endif

#ifdef __cplusplus
}
#endif

#endif /* PORTABLE_H */

