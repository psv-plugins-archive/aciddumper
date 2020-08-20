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
#include <psp2kern/kernel/sysmem.h>

#include <taihen.h>

#define USED __attribute__ ((used))
#define UNUSED __attribute__ ((unused))

#define GLZ(x) do {\
	if ((x) < 0) { goto fail; }\
} while (0)

#define HOOK_EXPORT(idx, mod, libnid, funcnid, func)\
	(hook_id[idx] = taiHookFunctionExportForKernel(\
		KERNEL_PID, hook_ref+idx, mod, libnid, funcnid, func##_hook))

#define N_HOOK 1
static SceUID hook_id[N_HOOK];
static tai_hook_ref_t hook_ref[N_HOOK];

static SceInt32 sceAppMgrDrmOpen_hook(const SceAppMgrDrmAddcontParam *pParam) {
	ksceDebugPrintf("%s%s\n", pParam->mountPoint.data, pParam->dirName.data);
	return TAI_NEXT(sceAppMgrDrmOpen_hook, hook_ref[0], pParam);
}

static void startup(void) {
	memset(hook_id, 0xFF, sizeof(hook_id));
	memset(hook_ref, 0xFF, sizeof(hook_ref));
}

static void cleanup(void) {
	for (int i = 0; i < N_HOOK; i++) {
		if (hook_id[i] >= 0) { taiHookReleaseForKernel(hook_id[i], hook_ref[i]); }
	}
}

USED int module_start(UNUSED SceSize args, UNUSED const void *argp) {
	startup();
	GLZ(HOOK_EXPORT(0, "SceAppMgr", 0xDCE180F8, 0xEA75D157, sceAppMgrDrmOpen));
	return SCE_KERNEL_START_SUCCESS;

fail:
	cleanup();
	return SCE_KERNEL_START_FAILED;
}

USED int module_stop(UNUSED SceSize args, UNUSED const void *argp) {
	cleanup();
	return SCE_KERNEL_STOP_SUCCESS;
}
