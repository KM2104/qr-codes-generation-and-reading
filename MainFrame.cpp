#include "MainFrame.h"
#include "conversion_to_qr_code.h"

#include <wx/clipbrd.h>
#include <wx/base64.h>
#include <wx/dataobj.h>
#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/mstream.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>

#include <ZXing/BarcodeFormat.h>
#include <ZXing/DecodeHints.h>
#include <ZXing/ImageView.h>
#include <ZXing/ReadBarcode.h>
#include <ZXing/Result.h>

#define HAS_ZXING_QR 1

enum
{
    ID_BUTTON_CLIPBOARD = wxID_HIGHEST + 1,
    ID_BUTTON_GENERATE,
    ID_BUTTON_SAVE,
    ID_BUTTON_COPY_PNG,
    ID_BUTTON_SHARE_EMAIL,
    ID_BUTTON_READ_FILE,
    ID_BUTTON_READ_SNIP
};

constexpr std::size_t MAX_QR_TEXT_UTF8_BYTES = 200;
constexpr long MAX_QR_TEXT_CHARS_UI = 200;

// Konstruktor g³ównego okna
MainFrame::MainFrame()
    : wxFrame(
        nullptr,
        wxID_ANY,
        "QR Code Generator",
        wxDefaultPosition,
        wxSize(760, 700))
{
    // Rejestracja handlerów obrazów wxWidgets (PNG/JPEG).
    // Dziêki temu SaveFile() potrafi zapisaæ obraz w odpowiednim formacie.
    wxInitAllImageHandlers();

    // G³ówny uk³ad pionowy: wejœcie, przyciski, status, podgl¹d.
    auto* rootSizer = new wxBoxSizer(wxVERTICAL);

    auto* inputLabel = new wxStaticText(this, wxID_ANY, "Tekst do zakodowania:");
    rootSizer->Add(inputLabel, 0, wxLEFT | wxRIGHT | wxTOP, 10);

    // Wielowierszowe pole tekstowe.
    // Uwaga edukacyjna: SetMaxLength() ogranicza liczbê znaków, a nie bajtów UTF-8.
    // Dlatego przed generowaniem i tak robimy walidacjê na bajtach.
    mInputCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    mInputCtrl->SetMaxLength(MAX_QR_TEXT_CHARS_UI);
    mInputCtrl->SetMinSize(wxSize(-1, 90));
    rootSizer->Add(mInputCtrl, 0, wxEXPAND | wxALL, 10);

    // Licznik d³ugoœci wpisywanego tekstu (znaki + bajty UTF-8).
    mCounterLabel = new wxStaticText(this, wxID_ANY, "Znaki: 0/200 | Bajty UTF-8: 0/200");
    rootSizer->Add(mCounterLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Pasek przycisków akcji zwi¹zanych z generowaniem i zapisem QR.
    auto* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* clipboardButton = new wxButton(this, ID_BUTTON_CLIPBOARD, "Tekst schowka");
    auto* generateButton = new wxButton(this, ID_BUTTON_GENERATE, "Generuj");
    auto* saveButton = new wxButton(this, ID_BUTTON_SAVE, "Zapisz (PNG/JPEG)");
    auto* copyPngButton = new wxButton(this, ID_BUTTON_COPY_PNG, "Kopiuj PNG");
    auto* shareButton = new wxButton(this, ID_BUTTON_SHARE_EMAIL, "Udostêpnij mailem");

    buttonSizer->Add(clipboardButton, 0, wxRIGHT, 8);
    buttonSizer->Add(generateButton, 0, wxRIGHT, 8);
    buttonSizer->Add(saveButton, 0, wxRIGHT, 8);
    buttonSizer->Add(copyPngButton, 0, wxRIGHT, 8);
    buttonSizer->Add(shareButton, 0);
    rootSizer->Add(buttonSizer, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Pasek przycisków akcji zwi¹zanych z odczytem QR.
    auto* readButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* readFromFileButton = new wxButton(this, ID_BUTTON_READ_FILE, "Odczytaj QR z pliku");
    auto* readFromSnipButton = new wxButton(this, ID_BUTTON_READ_SNIP, "Odczytaj z wycinka ekranu");
    readButtonSizer->Add(readFromFileButton, 0, wxRIGHT, 8);
    readButtonSizer->Add(readFromSnipButton, 0);
    rootSizer->Add(readButtonSizer, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Pole tylko do odczytu pokazuj¹ce tekst wyci¹gniêty z kodu QR.
    auto* decodedLabel = new wxStaticText(this, wxID_ANY, "Odczytana treœæ QR:");
    rootSizer->Add(decodedLabel, 0, wxLEFT | wxRIGHT, 10);
    mDecodedTextCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    mDecodedTextCtrl->SetMinSize(wxSize(-1, 80));
    rootSizer->Add(mDecodedTextCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Etykieta statusu aplikacji.
    // U¿ytkownik widzi, czy dana akcja zakoñczy³a siê powodzeniem czy b³êdem.
    mStatusLabel = new wxStaticText(this, wxID_ANY, "Status: Gotowy");
    rootSizer->Add(mStatusLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Podgl¹d wygenerowanego QR.
    mQrPreview = new wxStaticBitmap(this, wxID_ANY, wxBitmap(1, 1));
    mQrPreview->SetMinSize(wxSize(340, 340));
    rootSizer->Add(mQrPreview, 1, wxEXPAND | wxALL, 10);

    // Stopka informacyjna widoczna na dole okna aplikacji.
    auto* footerLabel = new wxStaticText(
        this,
        wxID_ANY,
        "Autor: KM2104 | Repo: github.com/KM2104/qr-codes-generation-and-reading");
    footerLabel->SetForegroundColour(wxColour(110, 110, 110));
    rootSizer->Add(footerLabel, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    SetSizer(rootSizer);

    // Podpinamy zdarzenia przycisków.
    Bind(wxEVT_BUTTON, &MainFrame::OnPasteFromClipboard, this, ID_BUTTON_CLIPBOARD);
    Bind(wxEVT_BUTTON, &MainFrame::OnGenerate, this, ID_BUTTON_GENERATE);
    Bind(wxEVT_BUTTON, &MainFrame::OnSaveQr, this, ID_BUTTON_SAVE);
    Bind(wxEVT_BUTTON, &MainFrame::OnCopyQrPngToClipboard, this, ID_BUTTON_COPY_PNG);
    Bind(wxEVT_BUTTON, &MainFrame::OnShareByEmail, this, ID_BUTTON_SHARE_EMAIL);
    Bind(wxEVT_BUTTON, &MainFrame::OnReadQrFromFile, this, ID_BUTTON_READ_FILE);
    Bind(wxEVT_BUTTON, &MainFrame::OnReadQrFromScreenSnip, this, ID_BUTTON_READ_SNIP);

    // Podpinamy zdarzenie zmiany tekstu, aby licznik odœwie¿a³ siê na bie¿¹co.
    mInputCtrl->Bind(wxEVT_TEXT, &MainFrame::OnInputTextChanged, this);

    // Ustawienie wartoœci pocz¹tkowej licznika i statusu.
    UpdateLengthCounter();
    UpdateStatus("Status: Wpisz tekst i kliknij 'Generuj'.", wxColour(0, 102, 204));
}

void MainFrame::OnPasteFromClipboard(wxCommandEvent& event)
{
    wxUnusedVar(event);

    if (!wxTheClipboard->Open())
    {
        UpdateStatus("Status: B³¹d otwarcia schowka.", *wxRED);
        wxMessageBox("Nie uda³o siê otworzyæ schowka.", "B³¹d", wxOK | wxICON_ERROR, this);
        return;
    }

    wxTextDataObject textData;
    if (wxTheClipboard->GetData(textData))
    {
        // SetValue() automatycznie wywo³a zdarzenie wxEVT_TEXT,
        // wiêc licznik odœwie¿y siê bez dodatkowego kodu.
        mInputCtrl->SetValue(textData.GetText());
        UpdateStatus("Status: Wczytano tekst ze schowka.", wxColour(0, 128, 0));
    }
    else
    {
        UpdateStatus("Status: W schowku brak tekstu.", wxColour(180, 120, 0));
        wxMessageBox("W schowku nie ma tekstu.", "Informacja", wxOK | wxICON_INFORMATION, this);
    }

    wxTheClipboard->Close();
}

void MainFrame::OnGenerate(wxCommandEvent& event)
{
    wxUnusedVar(event);

    wxString text = mInputCtrl->GetValue();
    text.Trim(true);
    text.Trim(false);

    if (text.IsEmpty())
    {
        UpdateStatus("Status: Brak danych do wygenerowania.", wxColour(180, 120, 0));
        wxMessageBox("Wpisz tekst lub wczytaj go ze schowka.", "Brak danych", wxOK | wxICON_WARNING, this);
        return;
    }

    const wxScopedCharBuffer utf8Buffer = text.utf8_str();
    const std::string inputText = utf8Buffer.data() != nullptr ? utf8Buffer.data() : "";

    if (inputText.size() > MAX_QR_TEXT_UTF8_BYTES)
    {
        UpdateStatus("Status: Tekst przekracza limit 200 bajtów UTF-8.", *wxRED);
        wxMessageBox("Tekst jest za d³ugi. Maksymalnie 200 bajtów UTF-8.", "Za d³ugi tekst", wxOK | wxICON_WARNING, this);
        return;
    }

    // Generujemy QR i zapamiêtujemy bitmapê jako "aktualn¹",
    // aby mo¿na j¹ by³o póŸniej zapisaæ lub udostêpniæ.
    mCurrentQrBitmap = GenerateQrCodeBitmap(inputText, 8, 4);
    mQrPreview->SetBitmap(mCurrentQrBitmap);
    Layout();

    UpdateStatus("Status: Kod QR wygenerowany poprawnie.", wxColour(0, 128, 0));
}

void MainFrame::OnSaveQr(wxCommandEvent& event)
{
    wxUnusedVar(event);

    if (!mCurrentQrBitmap.IsOk())
    {
        UpdateStatus("Status: Najpierw wygeneruj kod QR.", wxColour(180, 120, 0));
        wxMessageBox("Najpierw wygeneruj kod QR, a dopiero potem zapisz plik.", "Brak kodu QR", wxOK | wxICON_INFORMATION, this);
        return;
    }

    // Okno wyboru œcie¿ki i formatu pliku.
    // U¿ytkownik mo¿e zapisaæ QR jako PNG albo JPEG.
    wxFileDialog saveDialog(
        this,
        "Zapisz kod QR",
        "",
        "kod_qr.png",
        "Plik PNG (*.png)|*.png|Plik JPEG (*.jpg;*.jpeg)|*.jpg;*.jpeg",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() != wxID_OK)
    {
        UpdateStatus("Status: Zapis anulowany przez u¿ytkownika.", wxColour(120, 120, 120));
        return;
    }

    wxString filePath = saveDialog.GetPath();

    // Jeœli u¿ytkownik nie poda³ rozszerzenia, dok³adamy .png.
    wxFileName fileName(filePath);
    if (!fileName.HasExt())
    {
        fileName.SetExt("png");
        filePath = fileName.GetFullPath();
    }

    if (SaveCurrentQrToFile(filePath))
    {
        mLastSavedPath = filePath;
        UpdateStatus("Status: Zapisano plik: " + filePath, wxColour(0, 128, 0));
    }
    else
    {
        UpdateStatus("Status: Nie uda³o siê zapisaæ pliku.", *wxRED);
        wxMessageBox("Nie uda³o siê zapisaæ pliku QR.", "B³¹d zapisu", wxOK | wxICON_ERROR, this);
    }
}

void MainFrame::OnReadQrFromFile(wxCommandEvent& event)
{
    wxUnusedVar(event);

#if !HAS_ZXING_QR
    UpdateStatus("Status: Brak konfiguracji ZXing-C++ w projekcie.", *wxRED);
    wxMessageBox("Biblioteka ZXing-C++ nie jest skonfigurowana w projekcie.\nSkonfiguruj zale¿noœæ, aby w³¹czyæ odczyt QR.", "Brak ZXing-C++", wxOK | wxICON_WARNING, this);
    return;
#endif

    wxFileDialog openDialog(
        this,
        "Wybierz obraz z kodem QR",
        "",
        "",
        "Pliki obrazów (*.png;*.jpg;*.jpeg;*.bmp)|*.png;*.jpg;*.jpeg;*.bmp|Wszystkie pliki (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openDialog.ShowModal() != wxID_OK)
    {
        UpdateStatus("Status: Odczyt z pliku anulowany.", wxColour(120, 120, 120));
        return;
    }

    wxImage image;
    if (!image.LoadFile(openDialog.GetPath()))
    {
        UpdateStatus("Status: Nie uda³o siê wczytaæ obrazu z pliku.", *wxRED);
        wxMessageBox("Nie uda³o siê wczytaæ wskazanego pliku obrazu.", "B³¹d", wxOK | wxICON_ERROR, this);
        return;
    }

    wxString decodedText;
    if (DecodeQrFromImage(image, decodedText))
    {
        mDecodedTextCtrl->SetValue(decodedText);
        mQrPreview->SetBitmap(wxBitmap(image));
        UpdateStatus("Status: Odczytano kod QR z pliku.", wxColour(0, 128, 0));
    }
    else
    {
        mDecodedTextCtrl->SetValue("");
        UpdateStatus("Status: Nie znaleziono poprawnego kodu QR w pliku.", wxColour(180, 120, 0));
        wxMessageBox("Nie uda³o siê odczytaæ kodu QR z wybranego obrazu.", "Brak wyniku", wxOK | wxICON_INFORMATION, this);
    }
}

void MainFrame::OnReadQrFromScreenSnip(wxCommandEvent& event)
{
    wxUnusedVar(event);

#if !HAS_ZXING_QR
    UpdateStatus("Status: Brak konfiguracji ZXing-C++ w projekcie.", *wxRED);
    wxMessageBox("Biblioteka ZXing-C++ nie jest skonfigurowana w projekcie.\nSkonfiguruj zale¿noœæ, aby w³¹czyæ odczyt QR.", "Brak ZXing-C++", wxOK | wxICON_WARNING, this);
    return;
#endif

    // Uruchamiamy systemowy tryb wycinka ekranu (Windows).
    // Po wykonaniu wycinka obraz trafia do schowka i stamt¹d go odczytujemy.
    if (!wxLaunchDefaultBrowser("ms-screenclip:"))
    {
        UpdateStatus("Status: Nie uda³o siê uruchomiæ narzêdzia wycinka ekranu.", *wxRED);
        wxMessageBox("Nie uda³o siê uruchomiæ narzêdzia wycinka ekranu.", "B³¹d", wxOK | wxICON_ERROR, this);
        return;
    }

    const int answer = wxMessageBox(
        "Wykonaj wycinek ekranu z kodem QR.\n\n"
        "Po zakoñczeniu kliknij TAK, aby odczytaæ obraz ze schowka.",
        "Wycinek ekranu",
        wxYES_NO | wxICON_INFORMATION,
        this);

    if (answer != wxYES)
    {
        UpdateStatus("Status: Odczyt z wycinka anulowany.", wxColour(120, 120, 120));
        return;
    }

    wxImage image;
    if (!ReadImageFromClipboard(image))
    {
        UpdateStatus("Status: W schowku brak obrazu do odczytu QR.", wxColour(180, 120, 0));
        wxMessageBox("W schowku nie znaleziono obrazu. Wykonaj wycinek ponownie.", "Brak obrazu", wxOK | wxICON_INFORMATION, this);
        return;
    }

    wxString decodedText;
    if (DecodeQrFromImage(image, decodedText))
    {
        mDecodedTextCtrl->SetValue(decodedText);
        mQrPreview->SetBitmap(wxBitmap(image));
        UpdateStatus("Status: Odczytano kod QR z wycinka ekranu.", wxColour(0, 128, 0));
    }
    else
    {
        mDecodedTextCtrl->SetValue("");
        UpdateStatus("Status: Nie uda³o siê odczytaæ kodu QR z wycinka.", wxColour(180, 120, 0));
        wxMessageBox("Nie uda³o siê odczytaæ kodu QR z wycinka ekranu.", "Brak wyniku", wxOK | wxICON_INFORMATION, this);
    }
}

void MainFrame::OnShareByEmail(wxCommandEvent& event)
{
    wxUnusedVar(event);

    if (!mCurrentQrBitmap.IsOk())
    {
        UpdateStatus("Status: Najpierw wygeneruj kod QR.", wxColour(180, 120, 0));
        wxMessageBox("Najpierw wygeneruj kod QR, aby go udostêpniæ.", "Brak kodu QR", wxOK | wxICON_INFORMATION, this);
        return;
    }

    wxMemoryBuffer pngBuffer;
    if (!BuildCurrentQrPngBuffer(pngBuffer))
    {
        UpdateStatus("Status: Nie uda³o siê przygotowaæ PNG do e-maila.", *wxRED);
        wxMessageBox("Nie uda³o siê przygotowaæ pliku PNG do wys³ania e-mailem.", "B³¹d", wxOK | wxICON_ERROR, this);
        return;
    }

    // Tworzymy roboczy plik EML z gotowym za³¹cznikiem PNG.
    // Po otwarciu pliku EML domyœlny klient poczty powinien otworzyæ draft
    // z ju¿ osadzonym za³¹cznikiem kodu QR.
    wxFileName emlFile(wxStandardPaths::Get().GetTempDir(), "kod_qr_draft.eml");
    const wxString emlPath = emlFile.GetFullPath();

    const wxString boundary = "----=_QR_APP_BOUNDARY_0001";
    const wxString base64Png = wxBase64Encode(pngBuffer.GetData(), pngBuffer.GetDataLen());

    wxString emlContent;
    emlContent << "From: \"QR App\" <qr@app.local>\r\n";
    emlContent << "To: \r\n";
    emlContent << "Subject: Kod QR (PNG)\r\n";
    emlContent << "MIME-Version: 1.0\r\n";
    emlContent << "Content-Type: multipart/mixed; boundary=\"" << boundary << "\"\r\n";
    emlContent << "\r\n";
    emlContent << "--" << boundary << "\r\n";
    emlContent << "Content-Type: text/plain; charset=\"utf-8\"\r\n";
    emlContent << "Content-Transfer-Encoding: 8bit\r\n";
    emlContent << "\r\n";
    emlContent << "Wiadomosc z zalaczonym kodem QR w formacie PNG.\r\n";
    emlContent << "\r\n";
    emlContent << "--" << boundary << "\r\n";
    emlContent << "Content-Type: image/png; name=\"kod_qr.png\"\r\n";
    emlContent << "Content-Transfer-Encoding: base64\r\n";
    emlContent << "Content-Disposition: attachment; filename=\"kod_qr.png\"\r\n";
    emlContent << "\r\n";

    for (std::size_t pos = 0; pos < base64Png.length(); pos += 76)
    {
        emlContent << base64Png.Mid(pos, 76) << "\r\n";
    }

    emlContent << "--" << boundary << "--\r\n";

    wxFile file(emlPath, wxFile::write);
    if (!file.IsOpened())
    {
        UpdateStatus("Status: Nie uda³o siê utworzyæ draftu e-mail.", *wxRED);
        wxMessageBox("Nie uda³o siê utworzyæ pliku draftu wiadomoœci e-mail.", "B³¹d", wxOK | wxICON_ERROR, this);
        return;
    }

    if (!file.Write(emlContent))
    {
        UpdateStatus("Status: Nie uda³o siê zapisaæ draftu e-mail.", *wxRED);
        wxMessageBox("Nie uda³o siê zapisaæ pliku draftu wiadomoœci e-mail.", "B³¹d", wxOK | wxICON_ERROR, this);
        return;
    }

    file.Close();

    if (wxLaunchDefaultApplication(emlPath))
    {
        UpdateStatus("Status: Otworzono draft e-mail z za³¹cznikiem PNG.", wxColour(0, 128, 0));
    }
    else
    {
        UpdateStatus("Status: Nie uda³o siê otworzyæ klienta e-mail.", *wxRED);
        wxMessageBox("Nie uda³o siê uruchomiæ domyœlnego klienta poczty.", "B³¹d", wxOK | wxICON_ERROR, this);
    }
}

void MainFrame::OnCopyQrPngToClipboard(wxCommandEvent& event)
{
    wxUnusedVar(event);

    if (!mCurrentQrBitmap.IsOk())
    {
        UpdateStatus("Status: Najpierw wygeneruj kod QR.", wxColour(180, 120, 0));
        wxMessageBox("Najpierw wygeneruj kod QR, aby skopiowaæ obraz.", "Brak kodu QR", wxOK | wxICON_INFORMATION, this);
        return;
    }

    wxMemoryBuffer pngBuffer;
    if (!BuildCurrentQrPngBuffer(pngBuffer))
    {
        UpdateStatus("Status: Nie uda³o siê przygotowaæ PNG do schowka.", *wxRED);
        wxMessageBox("Nie uda³o siê przygotowaæ danych PNG.", "B³¹d", wxOK | wxICON_ERROR, this);
        return;
    }

    if (!wxTheClipboard->Open())
    {
        UpdateStatus("Status: B³¹d otwarcia schowka.", *wxRED);
        wxMessageBox("Nie uda³o siê otworzyæ schowka.", "B³¹d", wxOK | wxICON_ERROR, this);
        return;
    }

    auto* composite = new wxDataObjectComposite();

    // Umieszczamy dane PNG jako natywny format "PNG".
    // Dodatkowo dok³adamy bitmapê, aby zwiêkszyæ kompatybilnoœæ z aplikacjami,
    // które preferuj¹ standardowe formaty obrazu schowka systemowego.
    auto* pngDataObject = new wxCustomDataObject(wxDataFormat("PNG"));
    pngDataObject->SetData(pngBuffer.GetDataLen(), pngBuffer.GetData());
    composite->Add(pngDataObject, true);
    composite->Add(new wxBitmapDataObject(mCurrentQrBitmap));

    if (wxTheClipboard->SetData(composite))
    {
        UpdateStatus("Status: Skopiowano kod QR do schowka jako PNG.", wxColour(0, 128, 0));
    }
    else
    {
        delete composite;
        UpdateStatus("Status: Nie uda³o siê skopiowaæ PNG do schowka.", *wxRED);
        wxMessageBox("Nie uda³o siê skopiowaæ obrazu do schowka.", "B³¹d", wxOK | wxICON_ERROR, this);
    }

    wxTheClipboard->Close();
}

void MainFrame::OnInputTextChanged(wxCommandEvent& event)
{
    // To zdarzenie przychodzi przy ka¿dej zmianie w polu tekstowym
    // (pisanie, wklejanie, usuwanie, SetValue itp.).
    UpdateLengthCounter();

    // Przekazujemy zdarzenie dalej w ³añcuchu obs³ugi wxWidgets.
    // Dziêki temu nie blokujemy innych potencjalnych handlerów.
    event.Skip();
}

void MainFrame::UpdateLengthCounter()
{
    if (mInputCtrl == nullptr || mCounterLabel == nullptr)
    {
        return;
    }

    // Liczba znaków w wxString (warstwa UI / Unicode).
    const wxString text = mInputCtrl->GetValue();
    const std::size_t charCount = text.length();

    // Konwersja do UTF-8 i policzenie bajtów.
    // To bajty s¹ kluczowe dla realnego rozmiaru danych wejœciowych QR.
    const wxScopedCharBuffer utf8Buffer = text.utf8_str();
    const std::string utf8Text = utf8Buffer.data() != nullptr ? utf8Buffer.data() : "";
    const std::size_t byteCount = utf8Text.size();

    mCounterLabel->SetLabel(
        wxString::Format(
            "Znaki: %zu/%ld | Bajty UTF-8: %zu/%zu",
            charCount,
            MAX_QR_TEXT_CHARS_UI,
            byteCount,
            MAX_QR_TEXT_UTF8_BYTES));

    // Czerwony kolor sygnalizuje przekrocenie limitu bajtów.
    if (byteCount > MAX_QR_TEXT_UTF8_BYTES)
    {
        mCounterLabel->SetForegroundColour(*wxRED);
    }
    else
    {
        mCounterLabel->SetForegroundColour(wxNullColour);
    }
}

void MainFrame::UpdateStatus(const wxString& message, const wxColour& color)
{
    if (mStatusLabel == nullptr)
    {
        return;
    }

    mStatusLabel->SetLabel(message);

    if (color.IsOk())
    {
        mStatusLabel->SetForegroundColour(color);
    }
    else
    {
        mStatusLabel->SetForegroundColour(wxNullColour);
    }
}

bool MainFrame::SaveCurrentQrToFile(const wxString& filePath)
{
    if (!mCurrentQrBitmap.IsOk())
    {
        return false;
    }

    wxFileName fileName(filePath);
    const wxString ext = fileName.GetExt().Lower();

    // Dobieramy typ pliku na podstawie rozszerzenia.
    // Domyœlnie zapisujemy jako PNG (format bezstratny i najlepszy dla QR).
    wxBitmapType type = wxBITMAP_TYPE_PNG;
    if (ext == "jpg" || ext == "jpeg")
    {
        type = wxBITMAP_TYPE_JPEG;
    }

    wxImage image = mCurrentQrBitmap.ConvertToImage();
    return image.IsOk() && image.SaveFile(filePath, type);
}

bool MainFrame::BuildCurrentQrPngBuffer(wxMemoryBuffer& outputBuffer)
{
    outputBuffer.Clear();

    if (!mCurrentQrBitmap.IsOk())
    {
        return false;
    }

    wxImage image = mCurrentQrBitmap.ConvertToImage();
    if (!image.IsOk())
    {
        return false;
    }

    wxMemoryOutputStream memoryStream;
    if (!image.SaveFile(memoryStream, wxBITMAP_TYPE_PNG))
    {
        return false;
    }

    const size_t pngSize = memoryStream.GetSize();
    if (pngSize == 0)
    {
        return false;
    }

    outputBuffer.SetBufSize(pngSize);
    outputBuffer.SetDataLen(pngSize);
    memoryStream.CopyTo(outputBuffer.GetData(), pngSize);
    return true;
}

bool MainFrame::DecodeQrFromImage(const wxImage& image, wxString& decodedText)
{
    decodedText.clear();

#if !HAS_ZXING_QR
    wxUnusedVar(image);
    return false;
#else
    if (!image.IsOk() || image.GetData() == nullptr)
    {
        return false;
    }

    ZXing::ImageView source(image.GetData(), image.GetWidth(), image.GetHeight(), ZXing::ImageFormat::RGB);

    ZXing::DecodeHints hints;
    hints.setFormats(ZXing::BarcodeFormat::QRCode);
    hints.setTryHarder(true);

    const ZXing::Result result = ZXing::ReadBarcode(source, hints);
    if (!result.isValid())
    {
        return false;
    }

    const std::string text = result.text();
    decodedText = wxString::FromUTF8(text.c_str());
    if (decodedText.IsEmpty() && !text.empty())
    {
        decodedText = wxString(text);
    }

    return !decodedText.IsEmpty() || !text.empty();
#endif
}

bool MainFrame::ReadImageFromClipboard(wxImage& image)
{
    image = wxImage();

    if (!wxTheClipboard->Open())
    {
        return false;
    }

    wxBitmapDataObject bitmapData;
    const bool hasImage = wxTheClipboard->GetData(bitmapData);
    wxTheClipboard->Close();

    if (!hasImage)
    {
        return false;
    }

    wxBitmap bitmap = bitmapData.GetBitmap();
    if (!bitmap.IsOk())
    {
        return false;
    }

    image = bitmap.ConvertToImage();
    return image.IsOk();
}