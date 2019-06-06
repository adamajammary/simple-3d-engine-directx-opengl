#include "InputManager.h"

MouseState InputManager::mouseState;

int InputManager::Init()
{
	// MENU
	RenderEngine::Canvas.Window->Parent->Bind(wxEVT_MENU, &InputManager::OnIcon, ID_ICON_BROWSE, ID_ICON_LIGHT_SPOT);
	RenderEngine::Canvas.Window->Parent->Bind(wxEVT_MENU, &InputManager::OnList, ID_SCENE_CLEAR, ID_SCENE_SAVE);

	// KEYBOARD
	RenderEngine::Canvas.Window->Parent->Bind(wxEVT_KEY_DOWN, &InputManager::OnKeyboard);

	// MOUSE
	RenderEngine::Canvas.Canvas->Bind(wxEVT_MIDDLE_DOWN, &InputManager::OnMouseDown,    ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_RIGHT_DOWN,  &InputManager::OnMouseDown,    ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_LEFT_UP,     &InputManager::OnMouseUp,      ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_MIDDLE_UP,   &InputManager::OnMouseUp,      ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_RIGHT_UP,    &InputManager::OnMouseUp,      ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_MOTION,      &InputManager::OnMouseMove,    ID_CANVAS, ID_CANVAS);
	RenderEngine::Canvas.Canvas->Bind(wxEVT_MOUSEWHEEL,  &InputManager::OnMouseScroll,  ID_CANVAS, ID_CANVAS);

	// WINDOW
	RenderEngine::Canvas.Canvas->Bind(wxEVT_SIZE, &InputManager::OnWindowResize);

	return 0;
}

void InputManager::OnGraphicsMenu(wxCommandEvent &event)
{
	switch (event.GetId()) {
		case ID_ASPECT_RATIO:  RenderEngine::SetAspectRatio(event.GetString());      break;
		case ID_FOV:           RenderEngine::CameraMain->SetFOV(event.GetString());  break;
		case ID_DRAW_MODE:     RenderEngine::SetDrawMode(event.GetString());         break;
		case ID_DRAW_BOUNDING: RenderEngine::DrawBoundingVolume = event.IsChecked(); break;
		case ID_GRAPHICS_API:  RenderEngine::SetGraphicsAPI(event.GetString());      break;
		case ID_SRGB:          RenderEngine::EnableSRGB = event.IsChecked();         break;
		case ID_VSYNC:         RenderEngine::SetVSync(event.IsChecked());            break;
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
		SceneManager::LoadTerrain();
		break;
	case ID_ICON_WATER:
		SceneManager::LoadWater();
		break;
	case ID_ICON_LIGHT_DIRECTIONAL:
	case ID_ICON_LIGHT_POINT:
	case ID_ICON_LIGHT_SPOT:
		SceneManager::LoadLightSource(iconType);
		break;
	default:
		if ((iconType > ID_ICON_UNKNOWN) && (iconType < ID_CANVAS))
			SceneManager::LoadModel(Utils::RESOURCE_MODELS[iconType]);
		break;
	}
}

void InputManager::OnKeyboard(wxKeyEvent &event)
{
	bool result = false;

	if ((RenderEngine::CameraMain != nullptr) && !RenderEngine::Canvas.Window->IsPropertiesActive())
		result = RenderEngine::CameraMain->InputKeyboard(event.GetKeyCode());

	if (result)
		RenderEngine::Canvas.Window->UpdateProperties();
	else
		event.Skip();
}

void InputManager::OnList(wxCommandEvent &event)
{
	switch (event.GetId()) {
		case ID_COMPONENTS:
			SceneManager::SelectComponent(event.GetSelection());
			break;
		case ID_CHILDREN:
			SceneManager::SelectChild(event.GetSelection());
			break;
		case ID_SCENE_CLEAR:
			SceneManager::Clear();

			if (RenderEngine::CameraMain == nullptr) {
				SceneManager::AddComponent(new Camera());
				SceneManager::LoadLightSource(ID_ICON_LIGHT_DIRECTIONAL);
			}

			break;
		case ID_SCENE_LOAD:
			SceneManager::LoadScene(Utils::OpenFile(Utils::SCENE_FILE_FORMAT));
			break;
		case ID_SCENE_SAVE:
			SceneManager::SaveScene(Utils::SaveFile(Utils::SCENE_FILE_FORMAT));
			break;
		case ID_REMOVE_COMPONENT:
			SceneManager::RemoveSelectedComponent();
			break;
		case ID_REMOVE_CHILD:
			SceneManager::RemoveSelectedChild();
			break;
		case ID_RESET_CAMERA:
			if (RenderEngine::CameraMain != nullptr) {
				RenderEngine::CameraMain->Reset();
				RenderEngine::Canvas.Window->UpdateProperties();
			}
			break;
		default:
			throw;
	}
}

void InputManager::OnMouseDown(wxMouseEvent &event)
{
	InputManager::mouseState.Position = event.GetPosition();

	wxSetCursor(wxCursor(wxCURSOR_BLANK));
	RenderEngine::Canvas.Canvas->CaptureMouse();
}

void InputManager::OnMouseMove(wxMouseEvent &event)
{
	if (event.Dragging() && (event.MiddleIsDown() || event.RightIsDown()) && (RenderEngine::CameraMain != nullptr))
	{
		RenderEngine::CameraMain->InputMouseMove(event, InputManager::mouseState);

		wxPoint center = wxPoint(
			((RenderEngine::Canvas.Position.x + RenderEngine::Canvas.Size.GetWidth())  / 2),
			((RenderEngine::Canvas.Position.y + RenderEngine::Canvas.Size.GetHeight()) / 2)
		);

		RenderEngine::Canvas.Canvas->WarpPointer(center.x, center.y);
		InputManager::mouseState.Position = center;

		RenderEngine::Canvas.Window->UpdateProperties();
	}
}

void InputManager::OnMouseScroll(wxMouseEvent &event)
{
	if (RenderEngine::CameraMain != nullptr) {
		RenderEngine::CameraMain->InputMouseScroll(event);
		RenderEngine::Canvas.Window->UpdateProperties();
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

	if (event.GetButton() == wxMOUSE_BTN_LEFT)
		PhysicsEngine::CheckRayCasts(event);

	RenderEngine::Canvas.Window->DeactivateProperties();
}

void InputManager::OnPropertyChanged(wxPropertyGridEvent &event)
{
	if (event.GetId() == ID_SCENE_DETAILS)
		RenderEngine::Canvas.Window->UpdateComponents(event.GetProperty());
}

void InputManager::OnWindowResize(wxSizeEvent &event)
{
	RenderEngine::SetCanvasSize(RenderEngine::Canvas.Size.GetWidth(), RenderEngine::Canvas.Size.GetHeight());
}

void InputManager::Reset()
{
	if ((RenderEngine::Canvas.Window != nullptr) && (RenderEngine::Canvas.Window->Parent != nullptr))
	{
		// MENU
		RenderEngine::Canvas.Window->Parent->Bind(wxEVT_MENU, &InputManager::OnIcon, ID_ICON_BROWSE, ID_ICON_LIGHT_SPOT);
		RenderEngine::Canvas.Window->Parent->Bind(wxEVT_MENU, &InputManager::OnList, ID_SCENE_CLEAR, ID_SCENE_SAVE);

		// KEYBOARD
		RenderEngine::Canvas.Window->Parent->Unbind(wxEVT_KEY_DOWN, &InputManager::OnKeyboard);
	}

	if (RenderEngine::Canvas.Canvas != nullptr)
	{
		// MOUSE
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_MIDDLE_DOWN, &InputManager::OnMouseDown,    ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_RIGHT_DOWN,  &InputManager::OnMouseDown,    ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_LEFT_UP,     &InputManager::OnMouseUp,      ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_MIDDLE_UP,   &InputManager::OnMouseUp,      ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_RIGHT_UP,    &InputManager::OnMouseUp,      ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_MOTION,      &InputManager::OnMouseMove,    ID_CANVAS, ID_CANVAS);
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_MOUSEWHEEL,  &InputManager::OnMouseScroll,  ID_CANVAS, ID_CANVAS);

		// WINDOW
		RenderEngine::Canvas.Canvas->Unbind(wxEVT_SIZE,        &InputManager::OnWindowResize);
	}
}
