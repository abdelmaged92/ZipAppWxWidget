#include <wx/wx.h>

#include <wx/notebook.h>
#include "Zpanel.h"
#include "UZpanel.h"

class MyApp : public wxApp{
public:
	virtual bool OnInit();
};

wxIMPLEMENT_APP(MyApp);

class MyFrame : public wxFrame{
public:
	MyFrame(const wxString &title,const wxPoint &pos,const wxSize &size);
};

bool MyApp::OnInit(){
	MyFrame *frame=new MyFrame("Zip App - Abdelmaged Nour",wxDefaultPosition,wxDefaultSize);
	frame->Show(true);
	return true;
}

MyFrame::MyFrame(const wxString &title,const wxPoint &pos,const wxSize &size)
	: wxFrame(nullptr,wxID_ANY,title,pos,size){
	auto sizer=new wxBoxSizer(wxVERTICAL);

	auto tabs=new wxNotebook(this,wxID_ANY);
	tabs->SetInternalBorder(0);
	tabs->AddPage(new ZPanel(tabs),"Zip");
	tabs->AddPage(new UZPanel(tabs),"UnZip");

	sizer->Add(tabs,1,wxEXPAND|wxALL,FromDIP(10));
	this->SetSizer(sizer);
	this->SetSize(FromDIP(wxSize(700,600)));
	this->SetMinSize(FromDIP(wxSize(600,500)));
	this->SetBackgroundColour(tabs->GetBackgroundColour());
}
