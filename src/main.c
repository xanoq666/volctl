#include <psp2/types.h>
#include <psp2/ctrl.h>
#include <psp2/avconfig.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/modulemgr.h>

/* ---- Настройки ---- */
#define COMBO            (SCE_CTRL_LTRIGGER | SCE_CTRL_RTRIGGER) /* держать вместе */
#define BTN_UP           SCE_CTRL_UP     /* громче */
#define BTN_DOWN         SCE_CTRL_DOWN   /* тише  */
#define VOL_MIN          0
#define VOL_MAX          30
#define FIRST_DELAY_US   400000  /* пауза до автоповтора */
#define REPEAT_US        120000  /* интервал автоповтора */
#define POLL_US          20000
/* ------------------- */

static volatile int g_run = 1;
static SceUID g_thid = -1;

static void change_volume(int dir)
{
    int vol = 0;
    if (sceAVConfigGetSystemVol(&vol) < 0)
        return;
    vol += dir;
    if (vol < VOL_MIN) vol = VOL_MIN;
    if (vol > VOL_MAX) vol = VOL_MAX;
    sceAVConfigSetSystemVol(vol);
}

static int vol_thread(SceSize args, void *argp)
{
    SceCtrlData pad;
    unsigned int prev_dir = 0;
    uint64_t next = 0;

    while (g_run) {
        sceKernelDelayThread(POLL_US);

        if (sceCtrlPeekBufferPositive(0, &pad, 1) <= 0)
            continue;

        int dir = 0;
        if ((pad.buttons & COMBO) == COMBO) {
            if (pad.buttons & BTN_UP)        dir = +1;
            else if (pad.buttons & BTN_DOWN) dir = -1;
        }

        if (dir == 0) {
            prev_dir = 0;
            continue;
        }

        uint64_t now = sceKernelGetSystemTimeWide();
        if ((unsigned int)dir != prev_dir) {
            change_volume(dir);
            prev_dir = (unsigned int)dir;
            next = now + FIRST_DELAY_US;
        } else if (now >= next) {
            change_volume(dir);
            next = now + REPEAT_US;
        }
    }
    return 0;
}

int module_start(SceSize argc, const void *args)
{
    g_thid = sceKernelCreateThread("volctl", vol_thread, 0x10000100, 0x1000, 0, 0, NULL);
    if (g_thid >= 0)
        sceKernelStartThread(g_thid, 0, NULL);
    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
    g_run = 0;
    if (g_thid >= 0) {
        sceKernelWaitThreadEnd(g_thid, NULL, NULL);
        sceKernelDeleteThread(g_thid);
    }
    return SCE_KERNEL_STOP_SUCCESS;
}
