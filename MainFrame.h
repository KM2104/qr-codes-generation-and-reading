#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

#include <wx/wx.h>
#include <wx/statbmp.h>

// Klasa g³ównego okna aplikacji
class MainFrame : public wxFrame
{
public:
    MainFrame();

private:
    // Obs³uga klikniêcia przycisku pobieraj¹cego tekst ze schowka systemowego.
    void OnPasteFromClipboard(wxCommandEvent& event);

    // Obs³uga klikniêcia przycisku generowania kodu QR.
    void OnGenerate(wxCommandEvent& event);

    // Obs³uga zmiany tekstu w polu wejœciowym (aktualizacja licznika „na ¿ywo”).
    void OnInputTextChanged(wxCommandEvent& event);

    // Obs³uga zapisu aktualnie wygenerowanego kodu QR do pliku (PNG/JPEG).
    void OnSaveQr(wxCommandEvent& event);

    // Obs³uga kopiowania aktualnego QR do schowka jako PNG.
    void OnCopyQrPngToClipboard(wxCommandEvent& event);

    // Obs³uga udostêpniania kodu QR przez domyœlny klient poczty.
    void OnShareByEmail(wxCommandEvent& event);

    // Obs³uga odczytu kodu QR z pliku obrazu.
    void OnReadQrFromFile(wxCommandEvent& event);

    // Obs³uga odczytu kodu QR z wycinka ekranu zapisanego w schowku.
    void OnReadQrFromScreenSnip(wxCommandEvent& event);

    // Metoda pomocnicza aktualizuj¹ca etykietê z liczb¹ znaków i bajtów UTF-8.
    void UpdateLengthCounter();

    // Metoda pomocnicza aktualizuj¹ca etykietê statusu i jej kolor.
    void UpdateStatus(const wxString& message, const wxColour& color = wxNullColour);

    // Metoda pomocnicza zapisuj¹ca aktualn¹ bitmapê QR do wskazanego pliku.
    bool SaveCurrentQrToFile(const wxString& filePath);

    // Metoda pomocnicza koduj¹ca aktualn¹ bitmapê QR do pamiêci w formacie PNG.
    bool BuildCurrentQrPngBuffer(wxMemoryBuffer& outputBuffer);

    // Metoda pomocnicza dekoduj¹ca treœæ QR z obrazu przy u¿yciu ZXing-C++.
    bool DecodeQrFromImage(const wxImage& image, wxString& decodedText);

    // Metoda pomocnicza pobieraj¹ca obraz ze schowka systemowego.
    bool ReadImageFromClipboard(wxImage& image);

    // Pole, do którego u¿ytkownik wpisuje treœæ przeznaczon¹ do zakodowania w QR.
    wxTextCtrl* mInputCtrl = nullptr;

    // Podgl¹d wygenerowanego kodu QR w postaci bitmapy.
    wxStaticBitmap* mQrPreview = nullptr;

    // Etykieta pokazuj¹ca aktualn¹ d³ugoœæ wpisanego tekstu.
    // Pokazujemy jednoczeœnie liczbê znaków oraz bajtów UTF-8.
    wxStaticText* mCounterLabel = nullptr;

    // Etykieta statusu aplikacji (informacje o sukcesie/b³êdzie akcji u¿ytkownika).
    wxStaticText* mStatusLabel = nullptr;

    // Pole tylko do odczytu z tekstem odczytanym z kodu QR.
    wxTextCtrl* mDecodedTextCtrl = nullptr;

    // Ostatnio wygenerowana bitmapa QR.
    // Trzymamy j¹, aby mo¿na by³o j¹ póŸniej zapisaæ lub udostêpniæ.
    wxBitmap mCurrentQrBitmap;

    // Œcie¿ka ostatnio zapisanego pliku QR (przydatna przy udostêpnianiu mailem).
    wxString mLastSavedPath;
}; 
