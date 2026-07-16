#include <DX3D/EventBroadcasting/Parameters.h>

dx3d::Parameters::Parameters()
{
}

dx3d::Parameters::~Parameters()
{
}

void dx3d::Parameters::PutExtra(std::string paramName, uint32_t value)
{
	this->uint32Data[paramName] = (value);
}

uint32_t dx3d::Parameters::GetUInt32Extra(std::string paramName, uint32_t def_value)
{
	if (this->uint32Data.contains(paramName))
	{
		return this->uint32Data[paramName];
	}
	else
	{
		return def_value;
	}
}
