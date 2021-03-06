#include "iorx63n.h"
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

NOS_ISR(VECT_CMT0_CMI0)
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

void Timer_Init( void )
{
	/* Disable register protection */
    SYSTEM.PRCR.WORD = 0xA50B;

    /* Enable the CMT0 module */
    MSTP_CMT0 = 0;

    /* Re-enable register protection */
    SYSTEM.PRCR.BIT.PRKEY = 0xA5u;
    SYSTEM.PRCR.WORD &= 0xFF00u;

    /* Set CMT0 clock source as PLCK/512 */
    CMT0.CMCR.BIT.CKS = 0x2;

    /* Enable compare match interrupt */
    CMT0.CMCR.BIT.CMIE = 1;

    /* Enable CMT0 interrupt request */
    IEN(CMT0, CMI0) = 1;

    /* Set interrupt priority to 4 */
    IPR(CMT0, CMI0) = NOS_CONFIG_MAX_UNSAFE_ISR_PRIO;

    /* Set compare match to to generate debounce period */
    CMT0.CMCOR = 100;

    /* Reset count to zero */
    CMT0.CMCNT = 0x0000;

    /* Start timer */
    CMT.CMSTR0.BIT.STR0 = 1;
}

int main()
{
    nOS_Init();

    nOS_SemCreate(&semA, 0, 1);
    nOS_SemCreate(&semB, 0, 1);
    nOS_SemCreate(&semC, 0, 1);

    nOS_ThreadCreate(&threadA, ThreadA, 0, stackA, THREAD_STACK_SIZE, NOS_CONFIG_HIGHEST_THREAD_PRIO,   NOS_THREAD_READY);
    nOS_ThreadCreate(&threadB, ThreadB, 0, stackB, THREAD_STACK_SIZE, NOS_CONFIG_HIGHEST_THREAD_PRIO-1, NOS_THREAD_READY);
    nOS_ThreadCreate(&threadC, ThreadC, 0, stackC, THREAD_STACK_SIZE, NOS_CONFIG_HIGHEST_THREAD_PRIO-2, NOS_THREAD_READY);

    Timer_Init();

    __enable_interrupt();

    while (1) {
#if (NOS_CONFIG_TIMER_ENABLE > 0) && (NOS_CONFIG_TIMER_THREAD_ENABLE == 0)
        nOS_TimerProcess();
#endif
        nOS_SemGive(&semC);
    }
}
