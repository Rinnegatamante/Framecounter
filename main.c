#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <psp2/power.h>
#include <taihen.h>
#include <string.h>
#include "renderer.h"

#define SWITCH_MODE_DELAY 3000000
#define FPS_TIMER_TICK    1000000
#define NUM_MODES         2

enum {
	INTEGER_FPS,
	NO_FPS
};

static SceUID hook;
static tai_hook_ref_t display_ref;
uint64_t tick = 0;
uint64_t switch_tick = 0;
int frames = 0;
int fps = 0;
int mode = INTEGER_FPS;
uint64_t t_tick;

SceCtrlData pad;

int sceDisplaySetFrameBuf_patched(const SceDisplayFrameBuf *pParam, int sync) {
	sceCtrlPeekBufferPositive(0, &pad, 1);
	if (pad.buttons & SCE_CTRL_START){
		if (switch_tick == 0) switch_tick = sceKernelGetProcessTimeWide();
		else if ((sceKernelGetProcessTimeWide() - switch_tick) > SWITCH_MODE_DELAY){
			switch_tick = 0;
			t_tick = 0;
			tick = 0;
			frames = 0;
			mode = (mode + 1) % NUM_MODES;
		}
	}else switch_tick = 0;
	switch (mode) {
	case INTEGER_FPS:
		t_tick = sceKernelGetProcessTimeWide();
		updateFramebuf(pParam);
		if (tick == 0){
			tick = t_tick;
			setTextColor(0x00FFFFFF);
		}else{
			if ((t_tick - tick) > FPS_TIMER_TICK){
				fps = frames;
				frames = 0;
				tick = t_tick;
			}
			drawStringF(5, 5, "FPS: %d", fps);
		}
		frames++;
		break;
	default:
		break;
	}
	return TAI_CONTINUE(int, display_ref, pParam, sync);
}	

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {
	
	// Hooking sceDisplaySetFrameBuf
	hook = taiHookFunctionImport(&display_ref,
						TAI_MAIN_MODULE,
						TAI_ANY_LIBRARY,
						0x7A410B64,
						sceDisplaySetFrameBuf_patched);
						
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

	// Freeing hook
	taiHookRelease(hook, display_ref);

	return SCE_KERNEL_STOP_SUCCESS;
	
}