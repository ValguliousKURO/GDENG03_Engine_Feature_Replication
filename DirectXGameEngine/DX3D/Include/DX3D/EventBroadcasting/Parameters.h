#pragma once
#include <unordered_map>
#include <string>

namespace dx3d
{
	class Parameters
	{
	public:
		Parameters();
		~Parameters();

		void PutExtra(std::string paramName, uint32_t value);

		uint32_t GetUInt32Extra(std::string paramName, uint32_t def_value);

	private:
		std::unordered_map<std::string, uint32_t> uint32Data;
	};
}
