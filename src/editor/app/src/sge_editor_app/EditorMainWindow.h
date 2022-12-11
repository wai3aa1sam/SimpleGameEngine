#pragma once
#include <sge_editor.h>

namespace sge {

class EditorMainWindow : public NativeUIWindow {
	using Base = NativeUIWindow;
public:
	virtual void onCreate(CreateDesc& desc) override;
	virtual void onCloseButton() override;
	virtual void onUIMouseEvent(UIMouseEvent& ev) override;
	
	RenderContext& renderContext() { return *_renderContext; }

private:
	SPtr<RenderContext>	_renderContext;
};


}