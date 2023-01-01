#include <sge_editor_app-pch.h>
#include "EditorLayer.h"

namespace sge {

#if 0
#pragma mark --- EditorLayer_Base-Impl ---
#endif // 0	
#if 1

void EditorLayer_Base::update()
{
	onUpdate();
}

void EditorLayer_Base::render(RenderContext& rdCtx_, RenderData& rdData_)
{
	SGE_PROFILE_SCOPED;

	auto& rdCtx = rdCtx_; SGE_UNUSED(rdCtx);
	auto& rdReq = getRenderRequest(); SGE_UNUSED(rdReq);
	auto& rdData = rdData_; SGE_UNUSED(rdData);
	const auto& clientRect = rdData.clientRect; SGE_UNUSED(clientRect);

	class RenderScope
	{
	public:
		SGE_NODISCARD static RenderScope ctor(RenderContext& rdCtx_, RenderRequest& rdReq_, RenderData& rdData_) { return RenderScope(rdCtx_, rdReq_, rdData_); }

		~RenderScope()
		{
			SGE_PROFILE_SECTION("End Render");

			rdCtx.drawUI(rdReq);

			rdReq.swapBuffers();
			rdReq.commit();

			rdCtx.endRender();
		}
	private:
		RenderScope(RenderContext& rdCtx_, RenderRequest& rdReq_,  RenderData& rdData_)
			: 
			rdCtx(rdCtx_), rdReq(rdReq_), rdData(rdData_)
		{
			SGE_PROFILE_SECTION("Begin Render");

			const auto& clientRect = *rdData.clientRect;
			rdCtx.setFrameBufferSize(clientRect.size);
			rdCtx.beginRender();
			rdReq.reset(&rdCtx);
		}
		RenderContext& rdCtx;
		RenderRequest& rdReq;
		RenderData& rdData;
	};

	auto rdScope = RenderScope::ctor(rdCtx, rdReq, rdData);
	onRender(rdCtx, rdData);
}

#endif

#if 0
#pragma mark --- EditorLayer-Impl ---
#endif // 0	
#if 1

void EditorLayer::create()
{
	auto* renderer = Renderer::instance();
	auto* editor = EditorContext::instance(); SGE_UNUSED(editor)

	_camera.setPos(0,10,10);
	_camera.setAim(0,0,0);

	{
		Texture2D_CreateDesc texDesc;
		auto& image = texDesc.imageToUpload;

		// image.loadFile("Assets/Textures/uvChecker.png");
		// image.loadFile("Assets/Textures/uvChecker_BC1.dds");
		// image.loadFile("Assets/Textures/uvChecker_BC2.dds");
		// image.loadFile("Assets/Textures/uvChecker_BC3.dds");
		image.loadFile("Assets/Textures/uvChecker_BC7.dds");

		texDesc.size = image.size();
		texDesc.colorType = image.colorType();

		_testTexture = renderer->createTexture2D(texDesc);
	}

	{
		auto lineShader = renderer->createShader("Assets/Shaders/line.shader");
		_lineMaterial = renderer->createMaterial();
		_lineMaterial->setShader(lineShader);
	}

	{
		EngineContext::instance()->registerComponentType<CBoids>();
		_boidsEnt = _scene.addEntity("Boids");
		auto* cBoids = _boidsEnt->addComponent<CBoids>();
		cBoids->_boids.start();
	}

	{
		EngineContext::instance()->registerComponentType<CTerrain>();
		_terrainEnt = _scene.addEntity("Terrain");
		auto* cTerrain = _terrainEnt->addComponent<CTerrain>();
		
		my::Terrain_CreateDesc terrainDesc;
		Image img;
		img.loadPngFile("Assets/Terrain/TerrainTest/TerrainHeight_Small.png");

		terrainDesc.patchCount = { 8, 8 };

		terrainDesc.heightMap = &img;
		cTerrain->_terrain.create(terrainDesc);
	}
}

void EditorLayer::onUpdate()
{
	SGE_PROFILE_SCOPED;
	
	auto* cBoids = _boidsEnt->getComponent<CBoids>(); (void)cBoids;
	cBoids->_boids.update();
}

void EditorLayer::onRender(RenderContext& rdCtx_, RenderData& rdData_)
{
	SGE_PROFILE_SCOPED;

	auto& rdCtx = rdCtx_; SGE_UNUSED(rdCtx);
	auto& rdReq = getRenderRequest(); SGE_UNUSED(rdReq);
	auto& rdData = rdData_; SGE_UNUSED(rdData);
	const auto& clientRect = *rdData.clientRect; SGE_UNUSED(clientRect);
	
	_camera.setViewport(clientRect);
	rdReq.setCamera(_camera);

	rdReq.debug.drawBoundingBox = true;

	rdReq.lineMaterial = _lineMaterial;
	//		rdReq.matrix_model = Mat4f::s_identity();

	rdReq.clearFrameBuffers()->setColor({0, 0, 0.2f, 1});

#if 0
	{// debug culling
		auto fov = _camera.fov();
		_camera.setFov(fov / 2);
		rdReq.cameraFrustum = _camera.frustum();
		_camera.setFov(fov);
	}

	//-----
	//		auto time = GetTickCount() * 0.001f;
	//		auto s = abs(sin(time * 2));
	auto s = 1.0f;

	_material->setParam("test_float", s * 0.5f);
	_material->setParam("test_color", Color4f(s, s, s, 1));
	//------
	rdReq.drawFrustum(rdReq.cameraFrustum, Color4b(100, 255, 100, 255));

	rdReq.drawMesh(SGE_LOC, _renderMesh, _material);
	CRendererSystem::instance()->render(rdReq);

	//		_terrain.render(_renderRequest);

#else

	auto* cTerrain = _terrainEnt->getComponent<CTerrain>(); (void)cTerrain;
	//cTerrain->_terrain.render(rdReq);

	auto* cBoids = _boidsEnt->getComponent<CBoids>(); (void)cBoids;
	cBoids->_boids.render(rdReq);

#endif // 0

	_hierarchyWindow.draw(rdReq, _scene);
	_inspectorWindow.draw(rdReq, _scene);
	
	//ImGui::ShowDemoWindow(nullptr);
}

void EditorLayer::onUIMouseEvent(UIMouseEvent& ev) {

	if (ev.isDragging()) {
		using Button = UIMouseEventButton;
		switch (ev.pressedButtons) {
		case Button::Left: {
			auto d = ev.deltaPos * 0.01f;
			_camera.orbit(d.x, d.y);
		}break;

		case Button::Middle: {
			auto d = ev.deltaPos * 0.1f;
			_camera.move(d.x, d.y, 0);
		}break;

		case Button::Right: {
			auto d = ev.deltaPos * -0.1f;
			_camera.dolly(d.x + d.y);
		}break;
		}
	}
}

#endif

}