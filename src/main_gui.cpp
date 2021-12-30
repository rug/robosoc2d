// (c) 2021 Ruggero Rossi
#include "r2gui.h"
#include "simulator.h"
#include "simple_player.h"

using namespace std;
using namespace r2s;

class R2App: public wxApp{
    bool OnInit();
};

wxIMPLEMENT_APP(R2App);

bool R2App::OnInit(){
    r2s_gui::R2Launcher<SimplePlayer,SimplePlayer> *launcher=new r2s_gui::R2Launcher<SimplePlayer,SimplePlayer>();
    return true;
}

