#include "MKL25Z4.h"
#include "nOS.h"

#define TICKS_PER_SECOND            1000

#define THREAD_STACK_SIZE           128

nOS_Sem semA;
nOS_Sem semB;
nOS_Sem semC;
nOS_Thread threadA;
nOS_Thread threadB;
nOS_Thread threadC;
nOS_Stack stackA[THREAD_STACK_SIZE];
nOS_Stack stackB[THREAD_STACK_SIZE];
nOS_Stack stackC[THREAD_STACK_SIZE];

NOS_ISR(SysTick_Handler)
{
    nOS_Tick();
#if (NOS_CONFIG_TIMER_ENABLE > 0)
    nOS_TimerTick();
#endif
#if (NOS_CONFIG_TIME_ENABLE > 0)
    nOS_TimeTick();
#endif
}

void ThreadA (void *arg)
{
    NOS_UNUSED(arg);

    while (1) {
        nOS_SemTake(&semA, NOS_WAIT_INFINITE);
    }
}

void ThreadB (void *arg)
{
    NOS_UNUSED(arg);

    while (1) {
        nOS_SemTake(&semB, NOS_WAIT_INFINITE);
        nOS_SemGive(&semA);
    }
}

void ThreadC (void *arg)
{
    NOS_UNUSED(arg);

    while (1) {
        nOS_SemTake(&semC, NOS_WAIT_INFINITE);
        nOS_SemGive(&semB);
    }
}

int main()
{
    nOS_Init();

    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / TICKS_PER_SECOND);

    nOS_SemCreate(&semA, 0, 1);
    nOS_SemCreate(&semB, 0, 1);
    nOS_SemCreate(&semC, 0, 1);

    nOS_ThreadCreate(&threadA, ThreadA, 0, stackA, THREAD_STACK_SIZE, NOS_CONFIG_HIGHEST_THREAD_PRIO,   NOS_THREAD_READY);
    nOS_ThreadCreate(&threadB, ThreadB, 0, stackB, THREAD_STACK_SIZE, NOS_CONFIG_HIGHEST_THREAD_PRIO-1, NOS_THREAD_READY);
    nOS_ThreadCreate(&threadC, ThreadC, 0, stackC, THREAD_STACK_SIZE, NOS_CONFIG_HIGHEST_THREAD_PRIO-2, NOS_THREAD_READY);

    while (1) {
#if (NOS_CONFIG_TIMER_ENABLE > 0) && (NOS_CONFIG_TIMER_THREAD_ENABLE == 0)
        nOS_TimerProcess();
#endif
        nOS_SemGive(&semC);
    }
}
