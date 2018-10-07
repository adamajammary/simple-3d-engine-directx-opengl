#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_WINDOWFRAME_H
#define GE3D_WINDOWFRAME_H

class WindowFrame : public wxFrame
{
public:
	WindowFrame(const wxString &title, const wxPoint &pos, const wxSize &size, Window* parent);

public:
	Window*     Parent;
	wchar_t     Title[BUFFER_SIZE];
	wxCheckBox* VSyncEnable;

private:
	wxChoice*       dropDownDrawModes;
	wxChoice*       dropDownGraphicsAPIs;
	wxListBox*      listBoxComponents;
	wxListBox*      listBoxChildren;
	wxPropertyGrid* propertyGrid;
	wxBoxSizer*     sizerMiddle;
	wxBoxSizer*     sizerSceneDetails;

public:
	void     AddListComponent(Component* component);
	void     AddListChildren(std::vector<Mesh*> children);
	void     ClearScene();
	void     InitDetails();
	void     OnAbout(wxCommandEvent &event);
	void     OnExit(wxCommandEvent &event);
	void     RemoveComponent(int index);
	void     RemoveChild(int index);
	void     SelectComponent(int index);
	void     SelectChild(int index);
	wxString SelectedDrawMode();
	void     SetCanvas(wxGLCanvas* canvas);
	void     SetGraphicsAPI(int index);
	void     UpdateComponents(wxPGProperty* property);
	void     UpdateDetails();

private:
	void      addPropertyCheckbox(const wxString &label, const wxString &id, bool value);
	void      addPropertyEnum(const wxString &label, const wxString &id, const wxChar** values, const wxChar* value);
	void      addPropertyRange(const wxString &label, const wxString &id, float value, float min = 0.0f, float max = 1.0f, float step = 0.01f);
	void      addPropertyXYZ(const wxString &category, const wxString &id, float x, float y, float z, float min = 0.0f, float max = 1.0f, float step = 0.01f);
	void      setPropertyXYZ(const wxString &id, float x, float y, float z);
	glm::vec3 updatePropertyXYZ(const wxString &id, float value, const glm::vec3 &values);

	wxDECLARE_EVENT_TABLE();

};

#endif
