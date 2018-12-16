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
	wxCheckBox* sRGBEnable;
	wchar_t     Title[BUFFER_SIZE];
	wxCheckBox* VSyncEnable;

private:
	wxChoice*       dropDownDrawModes;
	wxChoice*       dropDownGraphicsAPIs;
	wxListBox*      listBoxComponents;
	wxListBox*      listBoxChildren;
	wxPropertyGrid* propertyGrid;
	wxBoxSizer*     sizerMiddle;
	wxBoxSizer*     sizerSceneProperties;

public:
	void     AddListComponent(Component* component);
	void     AddListChildren(std::vector<Component*> children);
	void     ClearScene();
	void     DeactivateProperties();
	int      InitProperties();
	bool     IsPropertiesActive();
	void     OnAbout(wxCommandEvent &event);
	void     OnExit(wxCommandEvent &event);
	void     RemoveComponent(int index);
	void     RemoveChild(int index);
	void     SelectComponent(int index);
	void     SelectChild(int index);
	wxString SelectedDrawMode();
	void     SetCanvas(wxGLCanvas* canvas);
	void     SetGraphicsAPI(int index);
	int      UpdateComponents(wxPGProperty* property);
	int      UpdateProperties();

private:
	void          addAds(wxBoxSizer* sizer);
	void          addButton(wxWindow* parent, void(*eventHandler)(wxCommandEvent &e), wxBoxSizer* sizer, IconType id, wxString label, int flag, const wxPoint &position = wxDefaultPosition, const wxSize &size = wxDefaultSize);
	wxCheckBox*   addCheckBox(wxGridBagSizer* sizer, IconType id, wxGBPosition position, bool default = false);
	wxChoice*     addDropDown(wxGridBagSizer* sizer, IconType id, wxGBPosition position, int nrOfChoices, const wxString* choices);
	void          addIcon(wxWindow* parent, wxBoxSizer* sizer, const Icon &icon, int flag);
	void          addLine(wxBoxSizer* sizer, int flag, wxSize size = wxSize(1, 1));
	wxListBox*    addListBox(wxBoxSizer* sizer, IconType id);
	void          addMenu();
	void          addPropertyCheckbox(const wxString &label, const wxString &id, bool value);
	void          addPropertyEnum(const wxString &label, const wxString &id, const wxChar** values, const wxChar* value);
	void          addPropertyRange(const wxString &label, const wxString &id, float value, float min = 0.0f, float max = 1.0f, float step = 0.01f);
	void          addPropertyXYZ(const wxString &category, const wxString &id, float x, float y, float z, float min = 0.0f, float max = 1.0f, float step = 0.01f);
	void          addTab(IconType id, const wxString &label, wxNotebook* tabs, const std::vector<Icon> &icons);
	wxStaticText* addTextLabel(wxString text);
	void          addTextLabel(wxGridBagSizer* sizer, wxString text, wxGBPosition position, int flag, int border = 0);
	void          addTextLabel(wxBoxSizer* sizer, wxString text, int flag, int border = 0);
	int           init();
	int           initPropertiesLightSources(Component* selected);
	void          setPropertyXYZ(const wxString &id, float x, float y, float z);
	glm::vec3     updatePropertyXYZ(const wxString &id, float value, const glm::vec3 &values);
	int           updatePropertiesLightSources(Component* selected);
	void          initPropertiesTextures(Component* selected);

	wxDECLARE_EVENT_TABLE();

};

#endif
