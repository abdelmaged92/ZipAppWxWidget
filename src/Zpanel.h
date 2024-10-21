#include <wx/wx.h>
#include <wx/filesys.h>
#include <wx/zipstrm.h>
#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/listctrl.h>
#include <wx/gbsizer.h>
#include <wx/progdlg.h>
#include <wx/filename.h>

using namespace std;

struct ZPanel : wxPanel{
	ZPanel(wxWindow *parent);
	wxBoxSizer *mainSizer;
	wxTextCtrl *dirToCompressText, *ZFileText;
	wxListView *filesList;
	void SetupDirectoryLoadSection();
	void SetupFileListSection();
	void SetupCompressSection();
	void LoadFilesToCompress();
	void PerformCompression();
};

ZPanel::ZPanel(wxWindow *parent) : wxPanel(parent){
	mainSizer=new wxBoxSizer(wxVERTICAL);
	SetupDirectoryLoadSection();
	SetupFileListSection();
	SetupCompressSection();
	SetSizer(mainSizer);
}

void ZPanel::SetupDirectoryLoadSection(){
	auto ZTitleLabel=new wxStaticText(this,wxID_ANY,"Zip Directory");
	ZTitleLabel->SetFont(wxFont(18,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD));
	dirToCompressText=new wxTextCtrl(this,wxID_ANY,wxEmptyString);
	dirToCompressText->SetEditable(false);
	auto dirButton=new wxButton(this,wxID_ANY,"Browse...");
	auto dirSectionSizer=new wxBoxSizer(wxHORIZONTAL);
	dirSectionSizer->Add(dirToCompressText,1,wxEXPAND|wxLEFT|wxBOTTOM,FromDIP(8));
	dirSectionSizer->Add(dirButton,0,wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM,FromDIP(8));
	mainSizer->AddSpacer(FromDIP(8));
	mainSizer->Add(ZTitleLabel,0,wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM,FromDIP(8));
	mainSizer->Add(dirSectionSizer,0,wxEXPAND);
	dirButton->Bind(wxEVT_BUTTON,[this](wxCommandEvent &event){
		wxDirDialog dialog(this,"Choose a directory",wxGetCwd(),wxDD_DEFAULT_STYLE|wxDD_DIR_MUST_EXIST);
		if(dialog.ShowModal()==wxID_OK){
			dirToCompressText->SetValue(dialog.GetPath());
			LoadFilesToCompress();
		}
	});
}

void ZPanel::SetupFileListSection(){
	auto filesLabel=new wxStaticText(this,wxID_ANY,"Files to compress");
	filesList=new wxListView(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL);
	filesList->AppendColumn("File");
	auto sizer=new wxGridBagSizer(FromDIP(8),FromDIP(8));
	sizer->AddGrowableCol(0);
	sizer->AddGrowableRow(1);
	sizer->Add(filesLabel,{0,0},{1,1},wxEXPAND);
	sizer->Add(filesList,{1,0},{1,1},wxEXPAND);
	mainSizer->Add(sizer,1,wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM,FromDIP(8));
	auto headerResize=[this](wxSizeEvent &event){
		wxListView *list=dynamic_cast<wxListView*>(event.GetEventObject());
		list->SetColumnWidth(0,list->GetClientSize().GetWidth());
		event.Skip();
	};
	filesList->Bind(wxEVT_SIZE,headerResize);
}

void ZPanel::SetupCompressSection(){
	auto outFileLabel=new wxStaticText(this,wxID_ANY,"Output file");
	outFileLabel->SetFont(wxFont(18,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD));
	ZFileText=new wxTextCtrl(this,wxID_ANY,wxEmptyString);
	ZFileText->SetEditable(false);
	auto outFileButton=new wxButton(this,wxID_ANY,"Change...");
	auto startButton=new wxButton(this, wxID_ANY, "Start");
	auto outFileSizer=new wxBoxSizer(wxHORIZONTAL);
	outFileSizer->Add(ZFileText,1,wxEXPAND|wxLEFT|wxBOTTOM,FromDIP(8));
	outFileSizer->Add(outFileButton,0,wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM,FromDIP(8));
	mainSizer->Add(outFileLabel,0,wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM,FromDIP(8));
	mainSizer->Add(outFileSizer,0,wxEXPAND);
	mainSizer->Add(startButton,0,wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM,FromDIP(8));
	outFileButton->Bind(wxEVT_BUTTON,[this](wxCommandEvent &event){
		wxFileDialog dialog(this,"Choose a file",wxGetCwd(),"out.zip","Zip files (*.zip)|*.zip",wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		if(dialog.ShowModal()==wxID_OK)
			ZFileText->SetValue(dialog.GetPath());
	});
	startButton->Bind(wxEVT_BUTTON,[this](wxCommandEvent &event){
		if(filesList->GetItemCount()==0)
			wxMessageBox("No files selected to compress","Error",wxOK|wxICON_ERROR);
		else if(ZFileText->GetValue().empty())
			wxMessageBox("No output file specified","Error",wxOK|wxICON_ERROR);
		else
			this->PerformCompression();
	});
}

struct IgnoringTraverser : wxDirTraverser{
	function<void(const string&)>fileEnterCallback;
	virtual wxDirTraverseResult OnFile(const wxString &filename) override{
		fileEnterCallback(filename.ToStdString());
		return wxDIR_CONTINUE;
	}
	virtual wxDirTraverseResult OnDir(const wxString &dirname) override{
		return wxDIR_CONTINUE;
	}
};

void ZPanel::LoadFilesToCompress(){
	const int PulseInterval=100;
	if(!wxDirExists(dirToCompressText->GetValue()))
		return;
	wxProgressDialog dialog("Loading files","Loading files to compress...",100,this,wxPD_APP_MODAL|wxPD_AUTO_HIDE);
	filesList->DeleteAllItems();
	IgnoringTraverser traverser;

	traverser.fileEnterCallback=[this,&dialog](const string &fileName){
		auto itemCount=filesList->GetItemCount();
		if(itemCount%PulseInterval==0)
			dialog.Pulse();
		filesList->InsertItem(itemCount, fileName);
	};
	auto dirToCompress=dirToCompressText->GetValue();
	wxDir(dirToCompress).Traverse(traverser);
}

void ZPanel::PerformCompression(){
	auto directoryToCompress=dirToCompressText->GetValue();
	auto ZFile=ZFileText->GetValue();
	auto outStream=wxFileOutputStream(ZFile);
	if(!outStream.IsOk()){
		wxMessageBox("Failed to open Z file","Error",wxOK|wxICON_ERROR);
		return;
	}
	wxZipOutputStream zip(outStream);
	wxProgressDialog dialog("Compressing","Compressing files...",filesList->GetItemCount(),this,wxPD_APP_MODAL|wxPD_AUTO_HIDE);
	int step=1;
	while(step*2<=filesList->GetItemCount()/50)step<<=1;
	for(int i=0;i<filesList->GetItemCount();i++){
		auto file=filesList->GetItemText(i);
		wxFileName fileName(file);
		fileName.MakeRelativeTo(directoryToCompress);
		auto relativePath=fileName.GetFullPath(wxPATH_NATIVE);
		zip.PutNextEntry(relativePath);
		wxFFileInputStream(file).Read(zip);
		zip.CloseEntry();
		if((i&(step-1))==0||i==filesList->GetItemCount()-1)dialog.Update(i);
	}
	zip.Close();
	outStream.Close();
}
