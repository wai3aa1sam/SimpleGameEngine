#pragma once

#include "EditorMainWindow.h"

namespace sge {

class EditorLayer;

class EditorApp : public NativeUIApp {
	using Base = NativeUIApp;
public:
	static EditorApp* instance() { return static_cast<EditorApp*>(Base::instance()); }

	virtual void onCreate(CreateDesc& desc) override;
	virtual void onRun() override;
	virtual void onQuit();

	void onUIMouseEvent(UIMouseEvent& ev);

			EditorMainWindow& mainWin()			{ return _mainWin; }
	const	EditorMainWindow& mainWin() const	{ return _mainWin; }

	const Rect2f& clientRect() const { return _mainWin.clientRect(); }

private:
	EditorMainWindow _mainWin;
	bool _shouldQuit = false;

	EditorLayer* _editorLayer = nullptr;
};

#if 1 // Temp

struct RenderData
{
	const Rect2f* clientRect = nullptr;
};

inline RenderRequest& getRenderRequest()
{
	// TODO: temp, get thread renderRequest
	static RenderRequest _renderRequest;

	return _renderRequest;
}

#endif // 1




}