# 内存管理

### **预览**

当任务或是信号量被创建时,内核需要进行动态的内存分配。
FreeRTOS带有pvPortMalloc(),vPortFree()实现范例。
小型嵌入式系统,通常是在启动调度之前的创建任务、队列和信号量。


### **内存分配方案**
heap_1.c:
    
    确定性的内存管理，实现了基本的pvPortMalloc(),且没有实现vPortFree()
    使用configTOTAL_HEAP_SIZE定义大小的简单数组，当调用pvPortMalloc()是，将数组进行简单地细分。
    如果不需要删除任务、队列或者信号量，则可能使用heap_1

heap_2.c：

    使用configTOTAL_HEAP_SIZE定义大小的简单数组
    使用最佳匹配算法保证,并支持内存释放。
    不会将空闲内存块合并成一个更大的内存块，因此会产生内存碎片。
    如果分配和释放总是相同大小的内存。

heap_3.c：

    简单的调用标准库函数malloc和free,但是通过暂时挂起调度器
    使得函数调用具备线程安全的特性。
    内存空间大小不受configTOTAL_HEAP_SIZE影响，而是由链接器配置
    决定。

heap_4.c：



heap_5.c：




五种管理方案：
source/portable/MemMang/heap_*.c