#pragma once
#include "EditorApp.h"

#include <sge_engine/feature/boids/Boids.h>
#include <sge_engine/feature/render/terrain/Terrain.h>

#if 0
#pragma mark --- XXXX-Impl ---
#endif // 0
#if 1

#endif

namespace sge {

class EditorLayer_Base : public NonCopyable
{
public:
	void update();
	void render(RenderContext& rdCtx_, RenderData& rdData_);

protected:
	virtual void onUpdate() = 0;
	virtual void onRender(RenderContext& rdCtx_, RenderData& rdData_) = 0;
	virtual void onRenderGUI(RenderContext& rdCtx_, RenderData& rdData_) = 0;

protected:
	EditorHierarchyWindow		_hierarchyWindow;
	EditorInspectorWindow		_inspectorWindow;
private:
	
};

class EditorLayer : public EditorLayer_Base
{
public:
	~EditorLayer()
	{
		SGE_LOG("~EditorLayer()");
	}
	void create();

	void onUIMouseEvent(UIMouseEvent& ev);

protected:
	virtual void onUpdate() override;
	virtual void onRender(RenderContext& rdCtx_, RenderData& rdData_) override;

	virtual void onRenderGUI(RenderContext& rdCtx_, RenderData& rdData_) override;

private:

	Math::Camera3f		_camera;
	Scene				_scene;

	SPtr<Shader>		_shader;

	SPtr<Shader>		_lineShader;
	SPtr<Material>		_lineMaterial;

	SPtr<Material>		_material;
	SPtr<Texture2D>		_testTexture;

	RenderMesh			_renderMesh;

	SPtr<MeshAsset>		_meshAsset;

	RenderTerrain		_terrain;

	Entity* _terrainEnt		= nullptr;
	Entity* _boidsEnt		= nullptr;
};

}