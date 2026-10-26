#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

#include <wx/wx.h>
#include "MainFrame.h"

// G³ówna klasa aplikacji
class MyApp : public wxApp
{
public:
    bool OnInit() override
    {
        auto* frame = new MainFrame();
        frame->Show(true);

        return true;
    }
};

// Uruchomienie aplikacji wxWidgets
wxIMPLEMENT_APP(MyApp);