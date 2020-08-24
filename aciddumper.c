/*
This file is part of Addcont ID Dumper
Copyright © 2020 浅倉麗子

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <string.h>

#include <psp2kern/appmgr.h>
#include <psp2kern/kernel/modulemgr.h>

#include <psp2dbg.h>
#include <taihen.h>

#define USED __attribute__ ((used))
#define UNUSED __attribute__ ((unused))

#define GLZ(x) do {\
	if ((x) < 0) { goto fail; }\
} while (0)

#define N_HOOK 1
static SceUID hook_id[N_HOOK];
static tai_hook_ref_t hook_ref[N_HOOK];

static SceUID hook_export(int idx, char *mod, int libnid, int funcnid, void *func) {
	SceUID ret = taiHookFunctionExportForKernel(KERNEL_PID, hook_ref+idx, mod, libnid, funcnid, func);
	if (ret >= 0) {
		SCE_DBG_LOG_INFO("Hooked %d UID %08X\n", idx, ret);
		hook_id[idx] = ret;
	} else {
		SCE_DBG_LOG_ERROR("Failed to hook %d error %08X\n", idx, ret);
	}
	return ret;
}
#define HOOK_EXPORT(idx, mod, libnid, funcnid, func)\
	hook_export(idx, mod, libnid, funcnid, func##_hook)

static int UNHOOK(int idx) {
	int ret = 0;
	if (hook_id[idx] >= 0) {
		ret = taiHookReleaseForKernel(hook_id[idx], hook_ref[idx]);
		if (ret == 0) {
			SCE_DBG_LOG_INFO("Unhooked %d UID %08X\n", idx, hook_id[idx]);
			hook_id[idx] = -1;
			hook_ref[idx] = -1;
		} else {
			SCE_DBG_LOG_ERROR("Failed to unhook %d UID %08X error %08X\n", idx, hook_id[idx], ret);
		}
	} else {
		SCE_DBG_LOG_WARNING("Tried to unhook %d but not hooked\n", idx);
	}
	return ret;
}

static SceInt32 sceAppMgrDrmOpen_hook(const SceAppMgrDrmAddcontParam *pParam) {
	SCE_DBG_LOG_INFO("%s%s\n", pParam->mountPoint.data, pParam->dirName.data);
	return TAI_NEXT(sceAppMgrDrmOpen_hook, hook_ref[0], pParam);
}

static void startup(void) {
	SCE_DBG_FILELOG_INIT("ux0:/aciddumper.log");
	memset(hook_id, 0xFF, sizeof(hook_id));
	memset(hook_ref, 0xFF, sizeof(hook_ref));
}

static void cleanup(void) {
	for (int i = 0; i < N_HOOK; i++) { UNHOOK(i); }
	SCE_DBG_FILELOG_TERM();
}

USED int module_start(UNUSED SceSize args, UNUSED const void *argp) {
	startup();
	GLZ(HOOK_EXPORT(0, "SceAppMgr", 0xDCE180F8, 0xEA75D157, sceAppMgrDrmOpen));
	SCE_DBG_LOG_INFO("module_start success\n");
	return SCE_KERNEL_START_SUCCESS;

fail:
	SCE_DBG_LOG_ERROR("module_start failed\n");
	cleanup();
	return SCE_KERNEL_START_FAILED;
}

USED int module_stop(UNUSED SceSize args, UNUSED const void *argp) {
	cleanup();
	return SCE_KERNEL_STOP_SUCCESS;
}
