#include "Window.h"

void Window::GameLoop(wxIdleEvent &event)
{
	event.RequestMore();

	TimeManager::UpdateFPS();
	PhysicsEngine::Update();
	RenderEngine::Draw();
}

bool Window::OnInit()
{
	// IMAGE HANDLERS
	wxInitAllImageHandlers();

	// WINDOW
	wxString title = wxString(Utils::APP_NAME).append(" ").append(Utils::APP_VERSION);

	this->frame = new WindowFrame(title, wxDefaultPosition, wxSize(1510, 860), this);
	this->frame->Show(true);
	//this->frame->Maximize(true);

	// RENDER ENGINE
	this->frame->SetStatusText("Initializing the Render Engine ...");

	int result = RenderEngine::Init(this->frame, wxSize(640, 360));

	if (result != 0)
	{
		wxMessageBox("ERROR: Failed to initialize the render engine.", this->frame->GetTitle().c_str(), wxOK | wxICON_ERROR);
		this->frame->SetStatusText("Initializing the Render Engine ... FAIL");

		return false;
	}

	// CAMERA
	this->frame->SetStatusText("Creating the Camera ...");

	Camera* camera = new Camera(
		glm::vec3(0.0f, 2.5f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), (glm::pi<float>() / 4.0f), 0.1f, 100.0f
	);

	if ((camera == nullptr) || !camera->IsValid())
	{
		wxMessageBox("ERROR: Failed to create the Camera.", this->frame->GetTitle().c_str(), wxOK | wxICON_ERROR);
		this->frame->SetStatusText("Creating the Camera ... FAIL");

		return false;
	}

	SceneManager::AddComponent(camera);

	// INPUT MANAGER
	this->frame->SetStatusText("Initializing the Input Manager ...");

	result = InputManager::Init();

	if (result != 0)
	{
		wxMessageBox("ERROR: Failed to initialize the Input Manager.", this->frame->GetTitle().c_str(), wxOK | wxICON_ERROR);
		this->frame->SetStatusText("Initializing the Input Manager ... FAIL");

		return false;
	}

	this->frame->SetStatusText(wxString("Successfully started ").append(Utils::APP_NAME));

	// START GAME LOOP
	TimeManager::Start();
	this->Connect(wxEVT_IDLE, wxIdleEventHandler(Window::GameLoop));

	return true;
}