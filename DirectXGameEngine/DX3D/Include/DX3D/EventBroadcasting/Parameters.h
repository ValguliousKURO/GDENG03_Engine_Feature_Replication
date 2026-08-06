#pragma once
#include <unordered_map>
#include <string>
#include <DX3D/Game/GameObject.h>
#include <DX3D/Math/Vec3.h>

namespace dx3d
{
	class Parameters
	{
	public:
		Parameters();
		~Parameters();

		void PutExtra(std::string paramName, uint32_t value);
		void PutExtra(std::string paramName, size_t value);
		void PutExtra(std::string paramName, bool value);
		void PutExtra(std::string paramName, GameObject* value);
		void PutExtra(std::string paramName, std::string value);
		void PutExtra(std::string paramName, const char* value);
		void PutExtra(std::string paramName, const Vec3& value);
		



		uint32_t GetUInt32Extra(std::string paramName, uint32_t def_value);
		size_t GetSizeTExtra(std::string paramName, size_t def_value);
		bool GetBoolExtra(std::string paramName, bool def_value);
		GameObject* GetGameObjectPtr(std::string paramName, GameObject* def_value);
		std::string GetStringExtra(std::string paramName, std::string def_value);
		Vec3 GetVec3Extra(std::string paramName, const Vec3& def_value);

	private:
		std::unordered_map<std::string, uint32_t> uint32Data;
		std::unordered_map<std::string, size_t> sizeTData;
		std::unordered_map<std::string, bool> boolData;
		std::unordered_map<std::string, GameObject*> ptrGameObjectData;
		
		std::unordered_map<std::string, std::string> stringData;
		
		std::unordered_map<std::string, Vec3> vec3Data;

		

	};
}
