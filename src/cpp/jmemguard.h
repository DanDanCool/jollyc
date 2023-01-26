#pragma once

namespace Jolly
{
	// overloads: & -> pointer to memory, * -> deference memory, () -> two stage init
	typedef <typename T>
	struct MemoryGuard<T>
	{

	};
}
