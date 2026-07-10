#include <DX3D/All.h>

class Camera : public dx3d::GameObject
{
	dx3d_typeid(Camera)
public:
	explicit Camera(const dx3d::GameObjectDesc& desc);
	virtual ~Camera() override;
protected:
	virtual void onCreate();
	virtual void onUpdate(dx3d::f32 deltaTime);
};