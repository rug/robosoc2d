// (c) 2021 Ruggero Rossi
// For compilers that support precompilation, includes "wx/wx.h".
//#include <wx/wxprec.h>
//#ifndef WX_PRECOMP
    #include <wx/wx.h>
    #include <wx/sizer.h>
    #include <wx/valnum.h>
//#endif

#include <chrono>
#include "simulator.h"
#include "simple_player.h"

using namespace std;
using namespace std::chrono;
using namespace r2s;

namespace r2s_gui { 

#define TITLE_TEXT "Very Simplified 2D Robotic Soccer Simulator"

constexpr unsigned int nplayers=4;
constexpr unsigned int default_random_seed = 42;

class R2Panel;
class GameThread;

class R2Frame: public wxFrame
{
public:
    R2Frame(const wxString& title, const wxPoint& pos, const wxSize& size,
       shared_ptr<R2Simulator> _simulator=buildSimulator<SimplePlayer,SimplePlayer>(nplayers, nplayers, defaultTeam1Name, defaultTeam2Name, createChronoRandomSeed() ));

    void DoStartThread(wxCommandEvent& event);
    void OnThreadUpdate(wxCommandEvent&);
    void OnThreadCompletion(wxCommandEvent&);
    void OnEraseBackGround(wxEraseEvent&);
    void OnClose(wxCloseEvent&);
    void setPanel(R2Panel *_panel){panel=_panel;}
    wxCriticalSection m_pThreadCS;    // protects the m_pThread pointer
    GameThread *m_pThread;

private:
    shared_ptr<R2Simulator> simulator;
    R2Panel *panel;
    int nPlayers1, nPlayers2;
    void OnStartGame(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnToggleShirts(wxCommandEvent& event);
    void OnSetSpeed1(wxCommandEvent& event);
    void OnSetSpeed2(wxCommandEvent& event);
    void OnSetSpeed3(wxCommandEvent& event);
    void OnSetSpeed4(wxCommandEvent& event);
    void OnSetSpeed5(wxCommandEvent& event);
    void OnSetSpeed6(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#define LAUNCHER_TITLE_TEXT "Launcher"

template <typename playerTeam1, typename playerTeam2>
class R2Launcher: public wxFrame
{
public:
    unsigned int random_seed;
    unsigned int players1;
    unsigned int players2;
    wxIntegerValidator<unsigned int>  val_seed;
    wxIntegerValidator<unsigned int>  val_players1;
    wxIntegerValidator<unsigned int>  val_players2;
    R2Launcher(const wxString& title=LAUNCHER_TITLE_TEXT, const wxPoint& pos=wxPoint(300,200), const wxSize& size=wxSize(300,300));
    wxTextCtrl *txtBoxPlayers1;
    wxTextCtrl *txtBoxPlayers2;
    wxTextCtrl *txtBoxSeed;
    wxButton *btLaunchSeed;
    wxButton *btLaunchRandom;
    void OnClose(wxCloseEvent&);
    void OnLaunchSeed( wxCommandEvent&);
    void OnLaunchRandom( wxCommandEvent&);
};

enum
{
LABEL_Seed= wxID_HIGHEST + 1,
TEXT_Seed, // declares an id which will be used to call our button
BUTTON_Launch_Seed,
BUTTON_Launch_Random
};

template <typename playerTeam1, typename playerTeam2>
R2Launcher <playerTeam1, playerTeam2>::R2Launcher(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE | wxFULL_REPAINT_ON_RESIZE),  val_seed(&random_seed, 0),
          val_players1(&players1, 0), val_players2(&players2, 0)
{

    CreateStatusBar();
    SetStatusText( TITLE_TEXT );

    wxStaticText *labelPlayers1=new wxStaticText 	(this, LABEL_Seed, "Number of players team 1", wxDefaultPosition, wxDefaultSize, 0, wxStaticTextNameStr);
    
    txtBoxPlayers1 = new wxTextCtrl(this, TEXT_Seed,
      std::to_string(nplayers).c_str(), wxDefaultPosition, wxDefaultSize,
      0, val_players1, wxTextCtrlNameStr);

    wxStaticText *labelPlayers2=new wxStaticText 	(this, LABEL_Seed, "Number of players team 2", wxDefaultPosition, wxDefaultSize, 0, wxStaticTextNameStr);

    txtBoxPlayers2 = new wxTextCtrl(this, TEXT_Seed,
      std::to_string(nplayers).c_str(), wxDefaultPosition, wxDefaultSize,
      0, val_players2, wxTextCtrlNameStr);

    wxStaticText *labelSeed=new wxStaticText 	(this, LABEL_Seed, "Seed", wxDefaultPosition, wxDefaultSize, 0, wxStaticTextNameStr);

    txtBoxSeed = new wxTextCtrl(this, TEXT_Seed,
      std::to_string(default_random_seed).c_str(), wxDefaultPosition, wxDefaultSize,
      0, val_seed, wxTextCtrlNameStr);

    btLaunchSeed = new wxButton(this, BUTTON_Launch_Seed,"Launch with this seed", wxDefaultPosition,wxDefaultSize);
    btLaunchRandom = new wxButton(this, BUTTON_Launch_Random,"Launch with instantly generated seed", wxDefaultPosition,wxDefaultSize);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(labelPlayers1, 1, wxEXPAND);
    sizer->Add(txtBoxPlayers1, 1, wxEXPAND);
    sizer->Add(labelPlayers2, 1, wxEXPAND);
    sizer->Add(txtBoxPlayers2, 1, wxEXPAND);
    sizer->Add(labelSeed, 1, wxEXPAND);
    sizer->Add(txtBoxSeed, 1, wxEXPAND);
    sizer->Add(btLaunchSeed, 1, wxEXPAND);
    sizer->Add(btLaunchRandom, 1, wxEXPAND);
    SetSizer(sizer);
    SetAutoLayout(true);
    Show();

    Bind(wxEVT_CLOSE_WINDOW, &R2Launcher<playerTeam1, playerTeam2>::OnClose, this);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &R2Launcher<playerTeam1, playerTeam2>::OnLaunchSeed, this, BUTTON_Launch_Seed);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &R2Launcher <playerTeam1, playerTeam2>::OnLaunchRandom, this, BUTTON_Launch_Random);
}

template <typename playerTeam1, typename playerTeam2>
void R2Launcher <playerTeam1, playerTeam2>::OnLaunchSeed( wxCommandEvent& event )
{
    if(   Validate() && TransferDataFromWindow() )
    {
      std::shared_ptr<R2Simulator> simulator=  buildSimulator<playerTeam1,playerTeam2>(players1, players2, defaultTeam1Name, defaultTeam2Name, random_seed);
      r2s_gui::R2Frame *frame = new r2s_gui::R2Frame( TITLE_TEXT, wxPoint(10,10), wxSize(960,700), simulator);
      frame->SetDoubleBuffered(true);
    }
}

template <typename playerTeam1, typename playerTeam2>
void R2Launcher<playerTeam1, playerTeam2>::OnLaunchRandom( wxCommandEvent& event )
{
  if(   Validate() && TransferDataFromWindow() )
    {
      std::shared_ptr<R2Simulator> simulator=  buildSimulator<playerTeam1,playerTeam2>(players1, players2, defaultTeam1Name, defaultTeam2Name, createChronoRandomSeed());
      r2s_gui::R2Frame *frame = new r2s_gui::R2Frame( TITLE_TEXT, wxPoint(10,10), wxSize(960,700), simulator);
      frame->SetDoubleBuffered(true);
    }
}

template <typename playerTeam1, typename playerTeam2>
void R2Launcher<playerTeam1, playerTeam2>::OnClose(wxCloseEvent&)
{
    Destroy();
}


} // end namespace