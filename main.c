#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <psp2/power.h>
#include <taihen.h>
#include "renderer.h"

static SceUID hook;
static tai_hook_ref_t display_ref;
uint64_t tick = 0;
int frames = 0;
int fps = 0;

int sceDisplaySetFrameBuf_patched(const SceDisplayFrameBuf *pParam, int sync) {
	uint64_t t_tick = sceKernelGetProcessTimeWide();
	updateFramebuf(pParam);
	if (tick == 0){
		tick = t_tick;
		setTextColor(0x00FFFFFF);
	}else{
		if ((t_tick - tick) > 1000000){
			fps = frames;
			frames = 0;
			tick = t_tick;
		}
		drawStringF(5, 5, "FPS: %d", fps);
	}
	frames++;
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