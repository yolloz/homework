#include "ContextSingleton.h"

namespace PlusPlusChat {
	ContextSingleton& ContextSingleton::GetInstance() {
		static ContextSingleton instance;
		return instance;
	}

	ContextSingleton::ContextSingleton() {}
}