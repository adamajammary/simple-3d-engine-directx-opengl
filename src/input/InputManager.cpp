#include "InputManager.h"

MouseState InputManager::mouseState;

int InputManager::Init()
{
	RenderEngine::Canvas.Window->Parent->Bind(wxEVT_KEY_DOWN, &InputManager::OnKeyboard);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_LEFT_DOWN,        &InputManager::OnMouseDown,    ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_MIDDLE_DOWN,      &InputManager::OnMouseDown,    ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_RIGHT_DOWN,       &InputManager::OnMouseDown,    ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_LEFT_UP,          &InputManager::OnMouseUp,      ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_MIDDLE_UP,        &InputManager::OnMouseUp,      ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_RIGHT_UP,         &InputManager::OnMouseUp,      ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_MOTION,           &InputManager::OnMouseMove,    ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_MOUSEWHEEL,       &InputManager::OnMouseScroll,  ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_SIZE,             &InputManager::OnWindowResize);

	return 0;
}

void InputManager::OnKeyboard(wxKeyEvent &event)
{
	bool result = false;

	if (RenderEngine::Canvas.Active && (RenderEngine::Camera != nullptr) && (event.GetEventType() == wxEVT_KEY_DOWN))
		result = RenderEngine::Camera->InputKeyboard(event.GetKeyCode());

	if (result)
		RenderEngine::Canvas.Window->UpdateDetails();
	else
		event.Skip();
}

void InputManager::OnMouseDown(wxMouseEvent &event)
{
	RenderEngine::Canvas.Active = (event.GetId() == ID_CANVAS);

	if (RenderEngine::Canvas.Active && (event.GetButton() == wxMOUSE_BTN_MIDDLE)) {
		InputManager::mouseState.Position = event.GetPosition();
		wxSetCursor(wxCursor(wxCURSOR_BLANK));
		RenderEngine::Canvas.Canvas->CaptureMouse();
	}
}

void InputManager::OnMouseMove(wxMouseEvent &event)
{
	if (event.Dragging() && event.MiddleIsDown() && (RenderEngine::Camera != nullptr))
	{
		RenderEngine::Camera->InputMouseMove(event, InputManager::mouseState);

		wxPoint center = wxPoint(
			((RenderEngine::Canvas.Position.x + RenderEngine::Canvas.Size.GetWidth())  / 2),
			((RenderEngine::Canvas.Position.y + RenderEngine::Canvas.Size.GetHeight()) / 2)
		);

		RenderEngine::Canvas.Canvas->WarpPointer(center.x, center.y);
		InputManager::mouseState.Position = center;
	}
}

void InputManager::OnMouseScroll(wxMouseEvent &event)
{
	if ((event.GetId() == ID_CANVAS) && (RenderEngine::Camera != nullptr)) {
		RenderEngine::Camera->InputMouseScroll(event);
		RenderEngine::Canvas.Window->UpdateDetails();
	} else {
		event.Skip();
	}
}

void InputManager::OnMouseUp(wxMouseEvent &event)
{
	if (RenderEngine::Canvas.Canvas->HasCapture()) {
		RenderEngine::Canvas.Canvas->ReleaseMouse();
		wxSetCursor(wxNullCursor);
	}

	if (RenderEngine::Canvas.Active && (event.GetButton() == wxMOUSE_BTN_LEFT))
		PhysicsEngine::CheckRayCasts(event);
}

void InputManager::OnGraphicsMenu(wxCommandEvent &event)
{
	switch (event.GetId()) {
		case ID_ASPECT_RATIO:  RenderEngine::SetAspectRatio(event.GetString());      break;
		case ID_FOV:           RenderEngine::Camera->SetFOV(event.GetString());      break;
		case ID_DRAW_MODE:     RenderEngine::SetDrawMode(event.GetString());         break;
		case ID_DRAW_BOUNDING: RenderEngine::DrawBoundingVolume = event.IsChecked(); break;
		case ID_GRAPHICS_API:  RenderEngine::SetGraphicsAPI(event.GetString());      break;
		case ID_VSYNC:         RenderEngine::SetVSYNC(event.IsChecked());            break;
	}
}

void InputManager::OnIcon(wxCommandEvent &event)
{
	IconType iconType = (IconType)event.GetId();

	switch (iconType) {
	case ID_ICON_BROWSE:
		SceneManager::LoadModel(Utils::OpenFile(Utils::MODEL_FILE_FORMATS));
		break;
	case ID_ICON_HUD:
		SceneManager::LoadHUD();
		break;
	case ID_ICON_SKYBOX:
		SceneManager::LoadSkybox();
		break;
	case ID_ICON_TERRAIN:
		SceneManager::LoadTerrain(10, 1, 2.0f);
		break;
	case ID_ICON_WATER:
		SceneManager::LoadWater();
		break;
	default:
		if ((iconType > ID_ICON_UNKNOWN) && (iconType < ID_CANVAS))
			SceneManager::LoadModel(Utils::RESOURCE_MODELS[iconType]);
		break;
	}
}

void InputManager::OnList(wxCommandEvent &event)
{
	switch (event.GetId()) {
		case ID_COMPONENTS:       SceneManager::SelectComponent(event.GetSelection());                break;
		case ID_CHILDREN:         SceneManager::SelectChild(event.GetSelection());                    break;
		case ID_SCENE_CLEAR:      SceneManager::Clear();                                              break;
		case ID_SCENE_LOAD:       SceneManager::LoadScene(Utils::OpenFile(Utils::SCENE_FILE_FORMAT)); break;
		case ID_SCENE_SAVE:       SceneManager::SaveScene(Utils::SaveFile(Utils::SCENE_FILE_FORMAT)); break;
		case ID_REMOVE_COMPONENT: SceneManager::RemoveSelectedComponent();                            break;
		case ID_REMOVE_CHILD:     SceneManager::RemoveSelectedChild();                                break;
	}
}

void InputManager::OnPropertyChanged(wxPropertyGridEvent &event)
{
	if (event.GetId() == ID_SCENE_DETAILS)
		RenderEngine::Canvas.Window->UpdateComponents(event.GetProperty());
}

void InputManager::Reset()
{
	if ((RenderEngine::Canvas.Window != nullptr) && (RenderEngine::Canvas.Window->Parent != nullptr))
		RenderEngine::Canvas.Window->Parent->Unbind(wxEVT_KEY_DOWN, &InputManager::OnKeyboard);

	if (RenderEngine::Canvas.Canvas != nullptr)
	{
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_LEFT_DOWN,   &InputManager::OnMouseDown,    ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_MIDDLE_DOWN, &InputManager::OnMouseDown,    ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_RIGHT_DOWN,  &InputManager::OnMouseDown,    ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_LEFT_UP,     &InputManager::OnMouseUp,      ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_MIDDLE_UP,   &InputManager::OnMouseUp,      ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_RIGHT_UP,    &InputManager::OnMouseUp,      ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_MOTION,      &InputManager::OnMouseMove,    ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_MOUSEWHEEL,  &InputManager::OnMouseScroll,  ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_SIZE,        &InputManager::OnWindowResize);
	}
}

void InputManager::OnWindowResize(wxSizeEvent &event)
{
	RenderEngine::SetCanvasSize(RenderEngine::Canvas.Size.GetWidth(), RenderEngine::Canvas.Size.GetHeight());
}
