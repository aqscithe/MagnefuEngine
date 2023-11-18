#include "EditorApp.h"
#include "EditorLayer.h"



Editor::Editor()
{
	PushLayer(new EditorLayer());
}

Editor::~Editor()
{

}


Magnefu::Application* Magnefu::CreateApplication()
{
	return new Editor();
}