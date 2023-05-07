#include "stdafx.h"
#include "ObjectHookManager.h"
#include "Tools.h"
#include "GameTime.h"
#include <VMP.h>

static ObjectHookManager* Manager = NULL;

static const uint32_t MAX_ANALYSIS = 200;
static void* VtableAnalysis[MAX_ANALYSIS];

void tryGetMissilePosition(void* obj) {

	__try {
		float x, y, z;
		BulletPositionGet(&(((war3::CMissile*)obj)->baseBullet.posData), x, y, z);
#ifndef _VMP
		OutputDebug("Missile Position: %.3f, %.3f, %.3f", x, y, z);
#endif
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{

	}	

}

void* __fastcall VtableCallAnalysis(void* obj, uint32_t index) {
	ObjectHookInfo* info = Manager->getObjectHookInfo(obj);
	if (info) {
#ifndef _VMP
		OutputDebug("%s method 0x%08X called: object = 0x%08X, index = %u, offset = 0x%X, time = %u", 
			RTTIClassNameGet(&(info->realVtable)), 
			info->realVtable[index],  
			obj, 
			index, 
			index * sizeof(void*),
			TimeRaw()
		);
#endif
		
		//tryGetMissilePosition(obj);
		
		return info->realVtable[index];
	} else {
#ifndef _VMP
		OutputDebug("Error: VtableCallAnalysis - No hook info found.");
#endif
	}
	return NULL; //������е��˴�����������˳�
}

static void* ObjectAddress;
template<uint32_t INDEX>
void __declspec(naked) AnalysisVtableMethod() { 
	__asm {
		push ebp;
		push edx;
		mov ebp, esp;
		mov ObjectAddress, ecx;
	}

	{
		void* realAddr = VtableCallAnalysis(ObjectAddress, INDEX); //Ϊ��ʹ��ģ�����
		__asm {
			mov eax, realAddr;
		}
	}

	__asm {
		mov esp, ebp;
		pop edx;
		pop ebp;
		mov ecx, ObjectAddress;
		jmp eax; 
	} 
}

template <uint32_t N>
void InitAnalysisVtable(void** vtable) {
	vtable[N - 1] = AnalysisVtableMethod<N - 1>;
	InitAnalysisVtable<N - 1>(vtable);
}

template <>
void InitAnalysisVtable<1>(void** vtable) {
	vtable[0] = AnalysisVtableMethod<0>;
}

ObjectHookManager::ObjectHookManager() {

}

void WriteVtable(void* obj, void** orginal) {
	__try {
		*(void***)(obj) = orginal;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
#ifdef _DEBUG
		void* dummy = orginal;
		OutputDebug("WriteVtable : Bad write ptr.(ObjectName: %s)", RTTIClassNameGet(&dummy));
#endif
	}	
}

ObjectHookManager::~ObjectHookManager() {
	ObjectHookInfoMapType::iterator iter;
	for (iter = this->map_.begin(); iter != this->map_.end(); ++iter) {
		if (iter->second.autoCleanup) {
			(void*)((uint32_t)iter->second.fakeVtable - 4);
		}
		WriteVtable(iter->first, iter->second.realVtable);
	}
}

ObjectHookInfo* ObjectHookManager::getObjectHookInfo(void* object) {
	ObjectHookInfo* rv = NULL;
	if (this->map_.count(object)) {
		rv = &(this->map_[object]);
	}
	return rv;	
}

void** ObjectHookManager::analysis(void* object, uint32_t vtableLength) {
	ObjectHookInfo* info = &(this->map_[object]);
	info->realVtable = *(void***)object;
	info->fakeVtable = VtableAnalysis;
	info->isDynamic = false;
	info->methodCount = vtableLength;
	info->autoCleanup = false;
	*(void***)object = info->fakeVtable;
	return info->realVtable;
}

void** ObjectHookManager::replace(void* object, uint32_t vtableLength, bool dynamic) {
	ObjectHookInfo* info = &(this->map_[object]);
	info->realVtable = *(void***)object;
	info->mem = new void*[vtableLength + 1]; //+1 for RTTI;
	info->fakeVtable = (void**)((uint32_t)info->mem + 4);
	info->isDynamic = dynamic;
	info->methodCount = vtableLength;
	info->autoCleanup = true;
	memcpy_s(info->mem, sizeof(void*) * (vtableLength + 1), (void*)((uint32_t)info->realVtable - 4), sizeof(void*) * (vtableLength + 1));
	*(void***)object = info->fakeVtable;
	return info->realVtable;
}

/*
void** ObjectHookManager::replace(void* object, void** vtable, uint32_t vtableLength) {
	ObjectHookInfo* info = &(this->map_[object]);
	info->realVtable = *(void***)object;
	info->fakeVtable = vtable;
	info->isDynamic = false;
	info->methodCount = vtableLength;
	info->autoCleanup = true;
	*(void***)object = info->fakeVtable;
	return info->realVtable;
}
*/

void* ObjectHookManager::apply(void* object, uint32_t offset, void* detour) {
	VMProtectBeginVirtualization("ObjectHookApply");
	if (this->map_.count(object)) {
		ObjectHookInfo* info = &(this->map_[object]);
		info->fakeVtable[offset / sizeof(void*)] = detour;
		return info->realVtable[offset / sizeof(void*)];
	}
	VMProtectEnd();
	return NULL;
}

void ObjectHookManager::cancel(void* object, uint32_t offset) {
	VMProtectBeginVirtualization("ObjectHookCancel");
	if (this->map_.count(object)) {
		ObjectHookInfo* info = &(this->map_[object]);
		info->fakeVtable[offset / sizeof(void*)] = this->getOrignal(object, offset);
	}
	VMProtectEnd();
}

void ObjectHookManager::restore(void* object) {
	VMProtectBeginVirtualization("ObjectHookRestore");
	if (this->map_.count(object)) {
		ObjectHookInfo* info = &(this->map_[object]);
		WriteVtable(object, info->realVtable);
		this->map_.erase(object);
	}
	VMProtectEnd();
}

void* ObjectHookManager::getOrignal(void* object, uint32_t offset) {
	void* rv = NULL;
	if (this->map_.count(object)) {
		rv = this->map_[object].realVtable[offset / sizeof(void*)];
	}
	return rv;
}

void ObjectHookManager::clearAllDynamic() {
	ObjectHookInfoMapType::iterator iter;
	for (iter = this->map_.begin(); iter != this->map_.end(); ) {
		ObjectHookInfo* info = &(iter->second);
		if (info->isDynamic) {
			delete [] iter->second.mem;
			iter = this->map_.erase(iter);
		} else
			++ iter;
	}
}

ObjectHookManager* GetObjectHookManager() {
	return Manager;
}

void ObjectHookManager_Init() {
	InitAnalysisVtable<MAX_ANALYSIS>(VtableAnalysis);

	Manager = new ObjectHookManager();
}

void ObjectHookManager_Cleanup() {
	delete Manager;
}