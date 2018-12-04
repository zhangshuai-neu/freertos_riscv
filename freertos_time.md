# FreeRTOS 的时间管理

FreeRTOS 的时间延迟有两种：相对模式 绝对模式

|函数|功能|描述|
|-|-|-|
|vTaskDelay|相对延迟||
|vTaskDelayUntil|绝对延迟|适用于周期性任务|
|-|-|-|

延迟等待列表