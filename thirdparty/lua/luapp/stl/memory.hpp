
#include "luapp/Config.hpp"

#if defined(_LUAPP_CPP11_) || defined( TOY_VC_2010)
	#include <memory>
#else
	#include "luapp/stl/sharedptr.hpp"
#endif
