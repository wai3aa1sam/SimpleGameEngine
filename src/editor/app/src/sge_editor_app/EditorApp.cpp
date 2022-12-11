#include <sge_editor_app-pch.h>
#include "EditorApp.h"

#include "EditorLayer.h"

namespace sge {

void EditorApp::onCreate(CreateDesc& desc)
{
	{
		String file = getExecutableFilename();
		String path = FilePath::dirname(file);
		path.append("/../../../../../../examples/Test101");

		auto* proj = ProjectSettings::instance();
		proj->setProjectRoot(path);
	}

#if 1 // compile shaders for quick testing
	{
		SHELLEXECUTEINFO ShExecInfo = {0};
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = L"open";
		ShExecInfo.lpFile = L"compile_shaders.bat";
		ShExecInfo.lpParameters = L"";
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_SHOW;
		ShExecInfo.hInstApp = NULL; 
		ShellExecuteEx(&ShExecInfo);
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
		CloseHandle(ShExecInfo.hProcess);
	}
#endif

	Base::onCreate(desc);

	{
		Renderer::CreateDesc renderDesc;
		//renderDesc.apiType = OpenGL;
		Renderer::create(renderDesc);

		EditorContext::createContext();

		//---
		NativeUIWindow::CreateDesc winDesc;
		winDesc.isMainWindow = true;
		_mainWin.create(winDesc);
		_mainWin.setWindowTitle("SGE Editor");
	}

	_editorLayer = new EditorLayer();
	_editorLayer->create();
}


void EditorApp::onRun()
{
	//Base::onRun();

	while (!_shouldQuit)
	{
		RenderData rdData;
		rdData.clientRect = &_mainWin.clientRect();

		_editorLayer->render(_mainWin.renderContext(), rdData);
		Base::pollEvent();
	}
}

void EditorApp::onQuit() 
{
	_shouldQuit = true;

	EditorContext::destroyContext();
	EngineContext::destroy();
	Base::onQuit();
}

void EditorApp::onUIMouseEvent(UIMouseEvent & ev)
{
	_editorLayer->onUIMouseEvent(ev);
}


}