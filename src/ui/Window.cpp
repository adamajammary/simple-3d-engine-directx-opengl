#include "Window.h"

void Window::GameLoop(wxIdleEvent &event)
{
	if (RenderEngine::Ready)
	{
		event.RequestMore();

		TimeManager::UpdateFPS();
		PhysicsEngine::Update();
		RenderEngine::Draw();
	}
}

int Window::OnExit()
{
	RenderEngine::Ready         = false;
	RenderEngine::Canvas.Canvas = nullptr;
	RenderEngine::Canvas.Window = nullptr;

	RenderEngine::Close();

	return 0;
}

bool Window::OnInit()
{
	// IMAGE HANDLERS
	wxInitAllImageHandlers();

	// WINDOW
	wxString title = wxString(Utils::APP_NAME).append(" ").append(Utils::APP_VERSION);

	this->frame = new WindowFrame(title, wxDefaultPosition, Utils::UI_WINDOW_SIZE, this);
	this->frame->Show(true);
	//this->frame->Maximize(true);

	// RENDER ENGINE
	this->frame->SetStatusText("Initializing the Render Engine ...");

	int result = RenderEngine::Init(this->frame, Utils::UI_RENDER_SIZE);

	if (result < 0)
	{
		wxMessageBox(("ERROR: Failed to initialize the render engine: " + std::to_wstring(result)), this->frame->GetTitle().c_str(), wxOK | wxICON_ERROR);
		this->frame->SetStatusText("Initializing the Render Engine ... FAIL");

		return false;
	}

	// CAMERA
	if ((RenderEngine::CameraMain == nullptr) || !RenderEngine::CameraMain->IsValid())
	{
		wxMessageBox("ERROR: Failed to create the Camera.", this->frame->GetTitle().c_str(), wxOK | wxICON_ERROR);
		this->frame->SetStatusText("ERROR: Failed to create the Camera.");

		return false;
	}

	// DIRECTIONAL LIGHT
	if ((SceneManager::LightSources[0] == nullptr) || !SceneManager::LightSources[0]->IsValid())
	{
		wxMessageBox("ERROR: Failed to create the directional light source.", this->frame->GetTitle().c_str(), wxOK | wxICON_ERROR);
		this->frame->SetStatusText("ERROR: Failed to create the directional light source.");

		return false;
	}

	// INPUT MANAGER
	this->frame->SetStatusText("Initializing the Input Manager ...");

	result = InputManager::Init();

	if (result < 0)
	{
		wxMessageBox(("ERROR: Failed to initialize the Input Manager: " + std::to_wstring(result)), this->frame->GetTitle().c_str(), wxOK | wxICON_ERROR);
		this->frame->SetStatusText("Initializing the Input Manager ... FAIL");

		return false;
	}

	this->frame->SetStatusText(wxString("Successfully started ").append(Utils::APP_NAME));

	// START GAME LOOP
	TimeManager::Start();
	this->Connect(wxEVT_IDLE, wxIdleEventHandler(Window::GameLoop));

	return true;
}
