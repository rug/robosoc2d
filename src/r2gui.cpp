// (c) 2021 Ruggero Rossi
#include "r2gui.h"

// For compilers that support precompilation, includes "wx/wx.h".
//#include <wx/wxprec.h>
//#ifndef WX_PRECOMP
    #include <wx/dcbuffer.h>
//#endif

#include <memory>
#include <iostream>
#include <algorithm>
#include <cstring>

#include <string> 
#include <thread>

#include "debug_print.h"

using namespace std;
using namespace r2s;

#define TITLE_TEXT "Very Simplified 2D Robotic Soccer Simulator"

namespace r2s_gui { 

constexpr long default_tick_duration1 = 300000;
constexpr long default_tick_duration2 = 200000;
constexpr long default_tick_duration3 = 100000;
constexpr long default_tick_duration4 = 50000;
constexpr long default_tick_duration5 = 25000; 
constexpr long default_tick_duration6 = 10000;
const double default_pitch_border = 3.0;

class Game {
private:
  vector<shared_ptr<R2Player>> teams[2];
  shared_ptr<R2Simulator> simulator;

public:
  R2EnvSettings sett;
  R2Pitch pitch;
  bool halfTime;

  Game(shared_ptr<R2Simulator> _simulator): simulator(_simulator) {

    R2GameState gameState = simulator->getGameState();
    sett=gameState.sett;
    pitch=gameState.pitch;
  }

  R2Environment getEnvironment(){
    return simulator->getGameState().env;
  }

  R2EnvSettings getSettings(){
    return sett;
  }

  R2Pitch getPitch(){
    return pitch;
  }

  string getStateString(){
    return simulator->getStateString();
  }

  unsigned int getSeed(){
    return simulator->getRandomSeed();
  }

  void playMatch(){
    simulator->playMatch();
  }

  bool stepIfPlaying(){
    return simulator->stepIfPlaying();
  }

};

// declare a new type of event, to be used by our MyThread class:
wxDECLARE_EVENT(wxEVT_COMMAND_MYTHREAD_COMPLETED, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_MYTHREAD_UPDATE, wxThreadEvent);

enum
{
    ID_StartGame = 1,
    GUITHREAD_EVENT
};

#define wxID_MENU_TOGGLESHIRTS (wxID_LOWEST-1)
#define wxID_MENU_SPEED1 (wxID_LOWEST-2)
#define wxID_MENU_SPEED2 (wxID_MENU_SPEED1-1)
#define wxID_MENU_SPEED3 (wxID_MENU_SPEED1-2)
#define wxID_MENU_SPEED4 (wxID_MENU_SPEED1-3)
#define wxID_MENU_SPEED5 (wxID_MENU_SPEED1-4)
#define wxID_MENU_SPEED6 (wxID_MENU_SPEED1-5)

class R2Frame;
class R2Panel;
class GameThread: public wxThread
{
protected:
  virtual wxThread::ExitCode Entry();

public:
  long tick_duration;
  Game game;
  R2EnvSettings sett;
  R2Pitch pitch;
  R2Environment env;
  R2Panel *panel;
  R2Frame *frame;
  char status_msg[256];
  wxCriticalSection dataCS;    // protects the simulation data
  wxCriticalSection speedCS;    // protects the speed variable
  GameThread( R2Frame *_frame, shared_ptr<R2Simulator> _simulator)
          : wxThread(wxTHREAD_DETACHED),  panel(NULL), frame(_frame), tick_duration(default_tick_duration3),
           game(_simulator) , sett(game.getSettings()), pitch(game.getPitch()), env() {}
  ~GameThread();
};

class R2Panel : public wxPanel
{
public:
    GameThread *m_pThread;
    bool showShirts;

    wxCriticalSection m_pThreadCS;    // protects the m_pThread pointer
    R2Panel(wxFrame* parent);
    void CalcBorders();
    void paintNow();
    void OnEraseBackGround(wxEraseEvent&);

private:
    char status_msg[256];
    wxCoord w, h;
    double w2, h2,ratioX, ratioY;
    double xLeft, xRight, yUp, yDown;
    wxCoord toXScreen(double x);
    wxCoord toYScreen(double y);
    void paintEvent(wxPaintEvent & evt);
    void render(wxDC& dc);
    void OnGUIThreadEvent(wxThreadEvent& event);
    wxDECLARE_EVENT_TABLE();
};


wxThread::ExitCode GameThread::Entry(){
  int duration=0;
  DEBUG_OUT("\n random seed:");
  DEBUG_OUT(game.getSeed());
  DEBUG_OUT("\n");
  char cseed[32];
  sprintf(cseed,"     random seed: %u",game.getSeed());

  high_resolution_clock::time_point t0 = high_resolution_clock::now(); 
  while(game.stepIfPlaying() && (!TestDestroy()) ){
    { wxCriticalSectionLocker enter(dataCS);
      env=game.getEnvironment();
      strcpy(status_msg, game.getStateString().c_str() ); }
      strcat(status_msg, cseed);
    
    // notify the gui it can draw
    wxMutexGuiEnter();
    if(panel){
      wxThreadEvent event( wxEVT_THREAD, GUITHREAD_EVENT );
      wxQueueEvent( panel, event.Clone() );
    }
    wxMutexGuiLeave();

    { wxCriticalSectionLocker enter(speedCS); // get the tick duration in case user changed it with GUI
      if(frame)
        duration= tick_duration;}

    DEBUG_OUT(status_msg);
    DEBUG_OUT("\n");

    high_resolution_clock::time_point t1 = high_resolution_clock::now(); 
    auto interval= std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    auto remaining = duration - interval;
    if(remaining > 0)
      std::this_thread::sleep_for(std::chrono::microseconds(remaining));
    t0 = high_resolution_clock::now();
  }

  wxQueueEvent(frame, new wxThreadEvent(wxEVT_COMMAND_MYTHREAD_COMPLETED));
  return (wxThread::ExitCode)0;     // success
}

GameThread::~GameThread()
{
    {wxCriticalSectionLocker enter(frame->m_pThreadCS);
    // the thread is being destroyed; make sure not to leave dangling pointers around
    frame->m_pThread = NULL;}

    {wxCriticalSectionLocker enter(panel->m_pThreadCS);
    panel->m_pThread = NULL;}
}

wxBEGIN_EVENT_TABLE(R2Frame, wxFrame)
    EVT_MENU(ID_StartGame,  R2Frame::DoStartThread)
    EVT_MENU(wxID_EXIT,  R2Frame::OnExit)
    EVT_MENU(wxID_ABOUT, R2Frame::OnAbout)
    EVT_MENU(wxID_MENU_TOGGLESHIRTS, R2Frame::OnToggleShirts)
    EVT_MENU(wxID_MENU_SPEED1, R2Frame::OnSetSpeed1)
    EVT_MENU(wxID_MENU_SPEED2, R2Frame::OnSetSpeed2)
    EVT_MENU(wxID_MENU_SPEED3, R2Frame::OnSetSpeed3)
    EVT_MENU(wxID_MENU_SPEED4, R2Frame::OnSetSpeed4)
    EVT_MENU(wxID_MENU_SPEED5, R2Frame::OnSetSpeed5)
    EVT_MENU(wxID_MENU_SPEED6, R2Frame::OnSetSpeed6)
    EVT_CLOSE(R2Frame::OnClose)
    EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_MYTHREAD_UPDATE, R2Frame::OnThreadUpdate)
    EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_MYTHREAD_COMPLETED, R2Frame::OnThreadCompletion)
    EVT_ERASE_BACKGROUND(R2Frame::OnEraseBackGround)
wxEND_EVENT_TABLE()
wxDEFINE_EVENT(wxEVT_COMMAND_MYTHREAD_COMPLETED, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_COMMAND_MYTHREAD_UPDATE, wxThreadEvent);

R2Frame::R2Frame(const wxString& title, const wxPoint& pos, const wxSize& size,
            shared_ptr<R2Simulator> _simulator)
        : wxFrame(NULL, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE | wxFULL_REPAINT_ON_RESIZE), panel(NULL), m_pThread(NULL),
          simulator(_simulator)
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_StartGame, "&Start Game...\tCtrl-S",
                     "Start a new game");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenu *menuOptions = new wxMenu;
    menuOptions->Append(wxID_MENU_TOGGLESHIRTS, _T("&Toggle shirts numbers\tCtrl-N"));
    wxMenu *menuSpeed = new wxMenu;
    menuSpeed->Append(wxID_MENU_SPEED1, _T("&Speed 1 A Third\tCtrl-1"));
    menuSpeed->Append(wxID_MENU_SPEED2, _T("&Speed 2 A Half\tCtrl-2"));
    menuSpeed->Append(wxID_MENU_SPEED3, _T("&Speed 3 NORMAL\tCtrl-3"));
    menuSpeed->Append(wxID_MENU_SPEED4, _T("&Speed 4 Double\tCtrl-4"));
    menuSpeed->Append(wxID_MENU_SPEED5, _T("&Speed 5 Fourfold\tCtrl-5"));
    menuSpeed->Append(wxID_MENU_SPEED6, _T("&Speed 6 Tenfold\tCtrl-6"));
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, "&File" );
    menuBar->Append( menuOptions, "&Options" );
    menuBar->Append( menuSpeed, "&Speed" );
    menuBar->Append( menuHelp, "&Help" );

    SetMenuBar( menuBar );
    CreateStatusBar();
    SetStatusText( TITLE_TEXT );

    panel = new R2Panel( (wxFrame*) this );
    panel->SetDoubleBuffered(true);
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(panel, 1, wxEXPAND);
    SetSizer(sizer);
    SetAutoLayout(true);
    Show();
}

void R2Frame::DoStartThread(wxCommandEvent& event)
{
    {wxCriticalSectionLocker enter(m_pThreadCS);
    if(m_pThread)
      return;
    m_pThread = new GameThread(this, simulator);}

    if ( m_pThread->Run() != wxTHREAD_NO_ERROR )
    {
        wxLogError("Can't create the thread!");
        delete m_pThread;
        m_pThread = NULL;
    }

    if(m_pThread &&panel)
    {
      panel->m_pThread=m_pThread;
      panel->CalcBorders();
      m_pThread->panel=panel;
    }
    // after the call to wxThread::Run(), the m_pThread pointer is "unsafe":
    // at any moment the thread may cease to exist (because it completes its work).
    // To avoid dangling pointers OnThreadExit() will set m_pThread
    // to NULL when the thread dies.
}

void R2Frame::OnExit(wxCommandEvent& event)
{
    Close( true );
}

void R2Frame::OnToggleShirts(wxCommandEvent& event)
{
    if(panel){
      panel->showShirts=!panel->showShirts;
    }
}

void R2Frame::OnSetSpeed1(wxCommandEvent& event)
{
  if(!m_pThread)
    return;

  { wxCriticalSectionLocker enter(m_pThread->speedCS);
    m_pThread->tick_duration=default_tick_duration1;}
}
void R2Frame::OnSetSpeed2(wxCommandEvent& event)
{
  if(!m_pThread)
    return;

  { wxCriticalSectionLocker enter(m_pThread->speedCS);
    m_pThread->tick_duration=default_tick_duration2;}
}
void R2Frame::OnSetSpeed3(wxCommandEvent& event)
{
  if(!m_pThread)
    return;

  { wxCriticalSectionLocker enter(m_pThread->speedCS);
    m_pThread->tick_duration=default_tick_duration3;}
}
void R2Frame::OnSetSpeed4(wxCommandEvent& event)
{
  if(!m_pThread)
    return;

  { wxCriticalSectionLocker enter(m_pThread->speedCS);
    m_pThread->tick_duration=default_tick_duration4;};
}
void R2Frame::OnSetSpeed5(wxCommandEvent& event)
{
  if(!m_pThread)
    return;

  { wxCriticalSectionLocker enter(m_pThread->speedCS);
    m_pThread->tick_duration=default_tick_duration5;}
}
void R2Frame::OnSetSpeed6(wxCommandEvent& event)
{
  if(!m_pThread)
    return;

  { wxCriticalSectionLocker enter(m_pThread->speedCS);
    m_pThread->tick_duration=default_tick_duration6;}
}

// this is needed empty to capture this event and do nothing, to advoid flickering.
void R2Frame::OnEraseBackGround(wxEraseEvent& event)
{}

void R2Frame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox( "(c) 2020 Ruggero Rossi",
                  TITLE_TEXT, wxOK | wxICON_INFORMATION );
}
void R2Frame::OnStartGame(wxCommandEvent& event)
{
    wxLogMessage("Start Game");
}


void R2Frame::OnThreadCompletion(wxCommandEvent&)
{
    //wxMessageOutputDebug().Printf("R2FRAME: GameThread exited!\n");
    DEBUG_OUT("\nR2FRAME: GameThread exited!\n");
}
void R2Frame::OnThreadUpdate(wxCommandEvent&)
{
    //wxMessageOutputDebug().Printf("R2FRAME: GameThread update...\n");
    //DEBUG_OUT("\nR2FRAME: GameThread update...\n");
}

void R2Frame::OnClose(wxCloseEvent&)
{
    {
        wxCriticalSectionLocker enter(m_pThreadCS);
        if (m_pThread)         // does the thread still exist?
        {
            //wxMessageOutputDebug().Printf("R2FRAME: deleting GameThread");
            DEBUG_OUT("R2FRAME: deleting GameThread\n");
            if (m_pThread->Delete() != wxTHREAD_NO_ERROR ){
              DEBUG_OUT("\nCan't delete the thread!\n");
                //wxLogError("Can't delete the thread!");
            }
        }
    }       // exit from the critical section to give the thread
            // the possibility to enter its destructor
            // (which is guarded with m_pThreadCS critical section!)
    while (1)
    {
        { // was the ~MyThread() function executed?
            wxCriticalSectionLocker enter(m_pThreadCS);
            if (!m_pThread) break;
        }
        // wait for thread completion
        wxThread::This()->Sleep(1);
    }
    Destroy();
}

BEGIN_EVENT_TABLE(R2Panel, wxPanel)
// catch paint events
EVT_PAINT(R2Panel::paintEvent)
EVT_THREAD(GUITHREAD_EVENT, R2Panel::OnGUIThreadEvent)
EVT_ERASE_BACKGROUND(R2Panel::OnEraseBackGround)
END_EVENT_TABLE()

R2Panel::R2Panel(wxFrame* parent) : wxPanel(parent),  w(0), h(0), m_pThread(NULL), showShirts(false) {}

void R2Panel::OnEraseBackGround(wxEraseEvent& event)
{}

void R2Panel::CalcBorders()
{
  wxCriticalSectionLocker enter(m_pThreadCS);

  if(m_pThread){  
    xLeft=fmin(m_pThread->game.pitch.xGoal2, m_pThread->game.pitch.x2-default_pitch_border);
    xRight=fmax(m_pThread->game.pitch.xGoal1, m_pThread->game.pitch.x1+default_pitch_border);
    yUp=m_pThread->game.pitch.y1+default_pitch_border;
    yDown=m_pThread->game.pitch.y2-default_pitch_border;
  }
}

wxCoord R2Panel::toXScreen(double x){
  x *=ratioX;
  x += w2; // center
  return static_cast<wxCoord>(x);
}

wxCoord R2Panel::toYScreen(double y){
  y *= -ratioY;
  y+=h2; //center

  return static_cast<wxCoord>(y);
}

void R2Panel::OnGUIThreadEvent(wxThreadEvent& event)
{
    Refresh();
}

void R2Panel::paintEvent(wxPaintEvent & evt)
{
    wxPaintDC dc(this);
    render(dc);
}

void R2Panel::paintNow()
{
    wxClientDC dc(this);
    render(dc);
}

/*
 * Actual rendering. 
 * In a separate
 * method so it works no matter what type of DC
 * (wxPaintDC or wxClientDC)
 */
void R2Panel::render(wxDC&  dc)
{
  if(!m_pThread)
  return;

  R2Environment lastEnv;
  { wxCriticalSectionLocker enter(m_pThread->dataCS);
    lastEnv=m_pThread->env;
    strcpy(status_msg, m_pThread->status_msg);}
  
  wxBufferedDC bdc(&dc);
  //bdc.GetSize (&w, &h);
  GetSize (&w, &h);
  int fh    = bdc.GetCharHeight()>>1;
  int fw    = bdc.GetCharWidth()>>1;

  double width=static_cast<double>(w);
  double height=static_cast<double>(h);
  w2=width/2;
  h2=height/2;
  ratioX=width/(xRight-xLeft);
  ratioY=height/(yUp-yDown);

  // blank all
  bdc.SetBrush(wxColor(0,128,0));
  bdc.DrawRectangle( 0, 0, w, h);
  
  int lineSize=static_cast<int>(m_pThread->game.sett.poleRadius*ratioY*2);
  // green grass
  bdc.SetBrush(*wxGREEN_BRUSH); // green filling
  bdc.SetPen( wxPen( wxColor(255,255,255), lineSize ) ); 
  auto leftX=toXScreen(m_pThread->game.pitch.x2);
  auto topY=toYScreen(m_pThread->game.pitch.y1);
  auto rightX=toXScreen(m_pThread->game.pitch.x1);
  auto bottomY=toYScreen(m_pThread->game.pitch.y2);
  bdc.DrawRectangle( leftX, topY, rightX-leftX, bottomY-topY);


  //goals

  bdc.SetBrush(wxColor(128,128,128));
  bdc.SetPen( wxPen( wxColor(200,200,200), lineSize ) ); 
  auto leftGoal=toXScreen(m_pThread->game.pitch.xGoal2);
  auto rightGoal=toXScreen(m_pThread->game.pitch.xGoal1);
  auto topGoal=toYScreen(m_pThread->game.pitch.yGoal1 + m_pThread->game.sett.poleRadius);
  auto bottomGoal=toYScreen(m_pThread->game.pitch.yGoal2 - m_pThread->game.sett.poleRadius);
  bdc.DrawRectangle( leftGoal, topGoal, leftX-leftGoal, bottomGoal-topGoal);
  bdc.DrawRectangle( rightX, topGoal, rightGoal-rightX, bottomGoal-topGoal);
  
  //areas
  bdc.SetBrush(*wxGREEN_BRUSH); // green filling
  bdc.SetPen( wxPen( wxColor(255,255,255), lineSize ) ); 
  auto areaLeft=toXScreen(m_pThread->game.pitch.areaLx);
  auto areaRight=toXScreen(m_pThread->game.pitch.areaRx);
  auto areaTop=toYScreen(m_pThread->game.pitch.areaUy);
  auto areaBottom=toYScreen(m_pThread->game.pitch.areaDy);
  bdc.DrawRectangle(leftX, areaTop, areaLeft-leftX, areaBottom-areaTop);
  bdc.DrawRectangle(areaRight, areaTop, rightX-areaRight, areaBottom-areaTop);

  //poles: uncomment to visualize poles (not really needed)
  
  /*
  bdc.SetPen(wxPen( wxColor(0,0,0), 1 ) ); 
  bdc.SetBrush(*wxWHITE_BRUSH); 
  int xDiameterPole=static_cast<int>(m_pThread->game.sett.poleRadius*ratioX*2);
  int yDiameterPole=static_cast<int>(m_pThread->game.sett.poleRadius*ratioY*2);
  int xRadiusPole=xDiameterPole/2;
  int yRadiusPole=yDiameterPole/2;
  auto poleX=toXScreen(m_pThread->game.pitch.poleLup.x);
  auto poleY=toYScreen(m_pThread->game.pitch.poleLup.y);
  bdc.DrawEllipse(poleX-xRadiusPole, poleY-yRadiusPole, xDiameterPole, yDiameterPole);
  poleX=toXScreen(m_pThread->game.pitch.poleLdown.x);
  poleY=toYScreen(m_pThread->game.pitch.poleLdown.y);
  bdc.DrawEllipse(poleX-xRadiusPole, poleY-yRadiusPole, xDiameterPole, yDiameterPole);
  poleX=toXScreen(m_pThread->game.pitch.poleRup.x);
  poleY=toYScreen(m_pThread->game.pitch.poleRup.y);
  bdc.DrawEllipse(poleX-xRadiusPole, poleY-yRadiusPole, xDiameterPole, yDiameterPole);
  poleX=toXScreen(m_pThread->game.pitch.poleRdown.x);
  poleY=toYScreen(m_pThread->game.pitch.poleRdown.y);
  bdc.DrawEllipse(poleX-xRadiusPole, poleY-yRadiusPole, xDiameterPole, yDiameterPole);
  */

  // center circle
  bdc.SetBrush(*wxGREEN_BRUSH); // green filling
  bdc.SetPen( wxPen( wxColor(255,255,255), lineSize ) ); 
  int xDiameter=static_cast<int>(m_pThread->game.sett.centerRadius*ratioX*2);
  int yDiameter=static_cast<int>(m_pThread->game.sett.centerRadius*ratioY*2);
  //dc.DrawEllipse(w2-xDiameter/2, h2-yDiameter/2, xDiameter, yDiameter);
  auto centerX=toXScreen(0.0);
  auto centerY=toYScreen(0.0);
  bdc.DrawEllipse(centerX-xDiameter/2, centerY-yDiameter/2, xDiameter, yDiameter);

  // half pitch line
  bdc.DrawLine(wxPoint(centerX,topY), wxPoint(centerX, bottomY-1));

  //draw players
  int xDiameterPlayer=static_cast<int>(m_pThread->game.sett.playerRadius*ratioX*2);
  int yDiameterPlayer=static_cast<int>(m_pThread->game.sett.playerRadius*ratioY*2);
  int xRadiusPlayer=xDiameterPlayer/2;
  int yRadiusPlayer=yDiameterPlayer/2;
  bdc.SetBrush(wxColor(32,32,255)); // goalkeeper is darker

  int i=0;
  bdc.SetTextForeground(wxColor(255,0,0)) ;

  for(auto p: lastEnv.teams[0]){
    bdc.SetPen(wxPen( wxColor(0,0,0), 1 ) ); 
    auto pX=toXScreen(p.pos.x);
    auto pY=toYScreen(p.pos.y);
    bdc.DrawEllipse(pX-xRadiusPlayer, pY-yRadiusPlayer, xDiameterPlayer, yDiameterPlayer);
    auto faceX=toXScreen(p.pos.x+ cos(p.direction)*m_pThread->game.sett.playerRadius );
    auto faceY=toYScreen(p.pos.y+ sin(p.direction)*m_pThread->game.sett.playerRadius );

    bdc.SetPen(wxPen( wxColor(255,255,0), 1 ) ); 
    bdc.DrawLine(wxPoint(pX, pY), wxPoint(faceX, faceY));
    bdc.SetBrush(wxColor(96,96,255));     

    if(showShirts){
      bdc.DrawText( to_string(i).c_str(), pX-fw, pY-fh );
      i++;
    }
  }

  i=0;
  bdc.SetTextForeground(wxColor(255,255,255)) ;

  bdc.SetBrush(wxColor(200,0,0)); // goalkeeper is darker
  bdc.SetPen(wxPen( wxColor(0,0,0), 1 ) ); 
  for(auto p: lastEnv.teams[1]){
    auto pX=toXScreen(p.pos.x);
    auto pY=toYScreen(p.pos.y);
    bdc.DrawEllipse(pX-xRadiusPlayer, pY-yRadiusPlayer, xDiameterPlayer, yDiameterPlayer);
    auto faceX=toXScreen(p.pos.x+ cos(p.direction)*m_pThread->game.sett.playerRadius );
    auto faceY=toYScreen(p.pos.y+ sin(p.direction)*m_pThread->game.sett.playerRadius );

    if(showShirts){
      bdc.DrawText( to_string(i).c_str(), pX-fw, pY-fh );
      i++;
    }

    bdc.DrawLine(wxPoint(pX, pY), wxPoint(faceX, faceY));
    bdc.SetBrush(wxColor(255,32,32)); 
  }

  //draw ball
  bdc.SetPen(wxPen( wxColor(0,0,0), 1 ) ); 
  bdc.SetBrush(*wxWHITE_BRUSH); 
  int xDiameterBall=static_cast<int>(m_pThread->game.sett.ballRadius*ratioX*2);
  int yDiameterBall=static_cast<int>(m_pThread->game.sett.ballRadius*ratioY*2);
  int xRadiusBall=xDiameterBall/2;
  int yRadiusBall=yDiameterBall/2;
  auto pX=toXScreen(lastEnv.ball.pos.x);
  auto pY=toYScreen(lastEnv.ball.pos.y);
  bdc.DrawEllipse(pX-xRadiusBall, pY-yRadiusBall, xDiameterBall, yDiameterBall);
  
  bdc.SetTextForeground(wxColor(255,255,255)) ;
  bdc.DrawText( status_msg, 8, 8 );
}

} // end namespace





