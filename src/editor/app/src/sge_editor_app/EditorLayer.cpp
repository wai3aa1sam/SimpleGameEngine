#include <sge_editor_app-pch.h>
#include "EditorLayer.h"

namespace sge {

#if 0
#pragma mark --- EditorLayer_Base-Impl ---
#endif // 0	
#if 1

void EditorLayer_Base::render(RenderContext& rdCtx_, RenderData& rdData_)
{
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

#if 1
		// image.loadFile("Assets/Textures/uvChecker.png");
		// image.loadFile("Assets/Textures/uvChecker_BC1.dds");
		// image.loadFile("Assets/Textures/uvChecker_BC2.dds");
		// image.loadFile("Assets/Textures/uvChecker_BC3.dds");
		image.loadFile("Assets/Textures/uvChecker_BC7.dds");

		texDesc.size = image.size();
		texDesc.colorType = image.colorType();

#else
		int w = 256;
		int h = 256;

		texDesc.size.set(w, h);
		texDesc.colorType = ColorType::RGBAb;

		image.create(Color4b::kColorType, w, h);

		for (int y = 0; y < w; y++) {
			auto span = image.row<Color4b>(y);
			for (int x = 0; x < h; x++) {
				span[x] = Color4b(	static_cast<u8>(x),
					static_cast<u8>(y), 
					0, 
					255);
			}
		}
#endif

		_testTexture = renderer->createTexture2D(texDesc);
	}

	{
		auto lineShader = renderer->createShader("Assets/Shaders/line.shader");
		_lineMaterial = renderer->createMaterial();
		_lineMaterial->setShader(lineShader);
	}

	{
		_shader = renderer->createShader("Assets/Shaders/test.shader");
		_material = renderer->createMaterial();
		_material->setShader(_shader);

		_material->setParam("mainTex", _testTexture);

		EditMesh editMesh;

#if 1
		WavefrontObjLoader::loadFile(editMesh, "Assets/Mesh/test.obj");
		// the current shader need color
		editMesh.addColors(Color4b(255, 255, 255, 255));

#else
		editMesh.pos.emplace_back( 0.0f,  0.5f, 0.0f);
		editMesh.pos.emplace_back( 0.5f, -0.5f, 0.0f);
		editMesh.pos.emplace_back(-0.5f, -0.5f, 0.0f);

		editMesh.color.emplace_back(255, 0, 0, 255);
		editMesh.color.emplace_back(0, 255, 0, 255);
		editMesh.color.emplace_back(0, 0, 255, 255);
#endif

		_renderMesh.create(editMesh);
	}

	{
		float size = 2048;
		float pos  = size / -2;
		float y    = -100;
		float height = 200;
		int maxLod = 6;
		_terrain.createFromHeightMapFile(
			Vec3f(pos, y, pos),
			Vec2f(size, size),
			height, 
			maxLod, 
			"Assets/Terrain/TerrainTest/TerrainHeight_Small.png");
	}

#if 0
	{ // ECS
		EditMesh editMesh;
		WavefrontObjLoader::loadFile(editMesh, "Assets/Mesh/box.obj");
		editMesh.addColors(Color4b(255, 255, 255, 255));

		_meshAsset = new MeshAsset();
		_meshAsset->mesh.create(editMesh);

		Vector<Entity*> entities;

		for (int i = 0; i < 25; i++) {
			auto* e = _scene.addEntity("Entity");
			auto* t = e->transform();

			entities.emplace_back(e);

			auto* mr = e->addComponent<CMeshRenderer>();
			mr->mesh = _meshAsset;

			auto mtl = renderer->createMaterial();
			mtl->setShader(_shader);
			mtl->setParam("test_color", Color4f(1, 1, 1, 1));
			mtl->setParam("mainTex", _testTexture);

			mr->material = mtl;

			const int col = 5;
			int x = i % col;
			int z = i / col;

			if (x == 0) {
				t->setLocalPos(0, 4, static_cast<float>(z));

			}
			else {
				auto* parent = entities[z * col]->transform();
				parent->addChild(e->transform());
				t->setLocalPos(static_cast<float>(x), 0, 0);
			}
		}

		//			editor->entitySelection.add(EntityId(1));
		editor->entitySelection.add(EntityId(3));
}
#endif // 0

	{
		EngineContext::instance()->registerComponentType<CBoids>();
		_boidsEnt = _scene.addEntity("Boids");
		auto* cBoids = _boidsEnt->addComponent<CBoids>();
		cBoids->_boids.start();
	}
}

void EditorLayer::onRender(RenderContext& rdCtx_, RenderData& rdData_)
{
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

	auto* cBoids = _boidsEnt->getComponent<CBoids>();
	cBoids->_boids.update();
	cBoids->_boids.render(rdReq);

#endif // 0




	_hierarchyWindow.draw(rdReq, _scene);
	_inspectorWindow.draw(rdReq, _scene);

	{
		auto win = EditorUI::Window("test");
		ImGui::Text(Fmt("Framerate: {}", ImGui::GetIO().Framerate).c_str());
	}
	
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