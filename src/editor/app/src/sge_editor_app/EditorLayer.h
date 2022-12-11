#pragma once
#include "EditorApp.h"

namespace sge {

class EditorLayer_Base : public NonCopyable
{
public:
	void render(RenderContext& rdCtx_, RenderData& rdData_);

protected:
	virtual void onRender(RenderContext& rdCtx_, RenderData& rdData_) = 0;
private:
	
};

class EditorLayer : public EditorLayer_Base
{
public:
	void create();

	void onUIMouseEvent(UIMouseEvent& ev);

protected:
	void onRender(RenderContext& rdCtx_, RenderData& rdData_);

private:
	SPtr<Shader>		_shader;

	SPtr<Material>		_lineMaterial;

	SPtr<Material>		_material;
	SPtr<Texture2D>		_testTexture;

	RenderMesh			_renderMesh;

	SPtr<MeshAsset>		_meshAsset;

	RenderTerrain		_terrain;

	Math::Camera3f		_camera;
	Scene				_scene;

	EditorHierarchyWindow		_hierarchyWindow;
	EditorInspectorWindow		_inspectorWindow;
};

}