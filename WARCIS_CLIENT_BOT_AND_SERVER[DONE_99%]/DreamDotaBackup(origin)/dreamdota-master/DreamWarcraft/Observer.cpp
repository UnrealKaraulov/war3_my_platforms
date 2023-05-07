#include "stdafx.h"
#include "Observer.h"

#include "Storm.h"
#include "Offsets.h"
#include "ObjectPool.h"

namespace CObserver {
	static void** OrginalVtable = NULL;

	void** getOriginalVtable() {
		return OrginalVtable;
	}

	war3::CObserver* create(EventProcessFunctionType func, void* userData) {
		war3::CObserver* t = Storm::MemAllocStruct<war3::CObserver>(userData);
		aero::generic_this_call<war3::CObserver*>(
			Offset(OBSERVER_CONSTRUCT),
			t
		);
		if (!OrginalVtable)
			OrginalVtable = VTBL(t);
		t->vtable = new void*[6];
		memcpy_s(t->vtable, sizeof(void*) * 6, OrginalVtable, sizeof(void*) * 6);

		if (NULL != func)
			setEventProcessFunction(t, func);
		return t;
	}

	void destory(war3::CObserver* t) {
		delete [] t->vtable;
		t->vtable = OrginalVtable;
		aero::generic_this_call<void>(
			VTBL(t)[1],
			t,
			1
		);
	}

	void setEventProcessFunction(war3::CObserver* t, EventProcessFunctionType func) {
		VTBL(t)[3] = func;
	}

	uint32_t addEventObserver(war3::CObserver* t, uint32_t evtId, war3::CObserver* ob) {
		return aero::generic_this_call<uint32_t>(
			VTBL(t)[2],
			t,
			evtId,
			evtId,	//TODO: ������ID�����ͬʱ���ڶ�����ʲô���ã�
			ob
		);
	}

	uint32_t dispatchEvent(war3::CObserver* t, war3::CEvent* evt) {
		return aero::generic_this_call<uint32_t>(
			VTBL(t)[4],
			t,
			evt
		);
	}
}

ObjectPool<Observer> ObserverPool(10);

OBSERVER_EVENT_HANDLER(DummyObserverEventHandler) {
	Observer* ob;
	if (NULL != (ob = (Observer*)Storm::MemGetUserData<war3::CObserver>(OBSERVER))) {
		if (ob->handler()) {
			ob->handler()(ob, EVENT->id);
			return 1;
		}
	}
	return 0;
}

Observer* Observer::Create(HandlerType handler) {
	Observer* rv = ObserverPool.get();
	rv->setBase(CObserver::create(DummyObserverEventHandler, rv));
	rv->setHandler(handler);
	return rv;
}

void Observer::Destroy(Observer* t) {
	if (t->base()) {
		CObserver::destory(t->base<war3::CObserver*>());
	}
	ObserverPool.ret(t);
}

Observer::Observer(war3::CObserver* base)  {
	this->setBase(base);
	this->handler_ = NULL;
}

void Observer::addEventObserver(Observer* child, uint32_t evtId) {
	war3::CObserver* t = this->base<war3::CObserver*>();
	war3::CObserver* c = child->base<war3::CObserver*>();
	if (t && c) {
		CObserver::addEventObserver(t, evtId, c); 
	}
}