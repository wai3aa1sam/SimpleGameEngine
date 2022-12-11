#include <sge_editor_app-pch.h>
#include "EditorMainWindow.h"

#include "EditorApp.h"

namespace sge {

void EditorMainWindow::onCreate(CreateDesc& desc) 
{
	SGE_DUMP_VAR(sizeof(Vertex_Pos));
	SGE_DUMP_VAR(sizeof(Vertex_PosColor));
	//		SGE_DUMP_VAR(sizeof(Vertex_PosColorUv));
	//		SGE_DUMP_VAR(sizeof(Vertex_PosColorUv2));

	Base::onCreate(desc);
	auto* renderer = Renderer::instance();
	auto* editor = EditorContext::instance(); SGE_UNUSED(editor)

	{
		RenderContext::CreateDesc renderContextDesc;
		renderContextDesc.window = this;
		_renderContext = renderer->createContext(renderContextDesc);
	}
}

void EditorMainWindow::onCloseButton()
{
	NativeUIApp::instance()->quit(0);
}

void EditorMainWindow::onUIMouseEvent(UIMouseEvent& ev) 
{
	if (_renderContext->onUIMouseEvent(ev))
		return;

	EditorApp::instance()->onUIMouseEvent(ev);
}

}